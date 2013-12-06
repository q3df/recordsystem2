#include "Recordsystem.h"
#include <cstdarg>

using namespace google::protobuf;
using namespace service;
using namespace std::placeholders;

PluginStore gPluginStore;

vmCvar_t rs_api_server;
vmCvar_t rs_api_port;
vmCvar_t rs_api_key;

extern "C" {
	void fix_utf8_string(std::string& str) {
		std::string temp;
		utf8::replace_invalid(str.begin(), str.end(), back_inserter(temp));
		str = temp;
	}

	const char *va( const char *format, ... ) {
		va_list		argptr;
		static char		string[2][32000];	// in case va is called by nested functions
		static int		index = 0;
		char	*buf;

		buf = string[index & 1];
		index++;

		va_start (argptr, format);
		vsprintf (buf, format,argptr);
		va_end (argptr);

		return buf;
	}
}

Recordsystem::Recordsystem(syscall_t syscall)
	: asyncExec_(new ApiAsyncExecuter()),
	  vm_syscall_(new Q3SysCall(syscall)),
	  syscall_(new Q3SysCall(syscall)) {
}

Recordsystem::~Recordsystem() {
	VmCvarItem *cvarItem = NULL;
	Q3EventHandler *eventItem = NULL;
	Q3User *userItem = NULL;

	delete asyncExec_;
	apiClient_->Close();
	delete Q3dfApi_;
	delete syscall_;
	delete vm_syscall_;

	while(!eventList_.empty()) {
		eventItem = eventList_.back();
		eventList_.pop_back();
		delete eventItem;
	}

	while(!cvarList_.empty()) {
		cvarItem = cvarList_.back();
		cvarList_.pop_back();
		delete cvarItem;
	}

	while(!userList_.empty()) {
		userItem = userList_.back();
		userList_.pop_back();
		delete userItem;
	}

	eventList_.clear();
	cvarList_.clear();
	pluginList_.clear();
	userList_.clear();
}

ApiAsyncExecuter *Recordsystem::GetAsyncExecuter() {
	return asyncExec_;
}

Q3dfApi_Stub *Recordsystem::GetQ3dfApi() {
	return Q3dfApi_;
}

Q3SysCall *Recordsystem::GetVmSyscalls() {
	return vm_syscall_;
}

Q3SysCall *Recordsystem::GetSyscalls() {
	return syscall_;
}

void Recordsystem::AddEventHandler(Q3EventHandler *eventItem) {
	eventList_.push_back(eventItem);
}

void Recordsystem::RemoveEventHandler(Q3EventHandler *eventItem) {
	GetSyscalls()->PrintError("Recordsystem::RemoveHook not implemented!");
}

int Recordsystem::VmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	int ret = 0;
	PluginBase *pBase;
	Q3EventHandler *eventItem = NULL;
	if(eventItem == NULL)
		eventItem = NULL; // fixed compile error because of unused...


	// ***************************************************
	// vmMain before forward to vm
	// ***************************************************
	switch(command) {
	case GAME_INIT:
		if(!GameInit(arg0, arg1, arg2))
			return 0;

		EXECUTE_EVENT_VOID_ARG12(command, EXECUTE_TYPE_BEFORE, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		break;

	case GAME_RUN_FRAME:
		UpdateQuake3Cvars();
		EXECUTE_EVENT_VOID_ARG12(command, EXECUTE_TYPE_BEFORE, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		GetAsyncExecuter()->DoMainThreadWork();
		break;

	case GAME_SHUTDOWN:
		EXECUTE_EVENT_VOID_ARG12(command, EXECUTE_TYPE_BEFORE, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)

		while(!pluginList_.empty()) {
			pBase = pluginList_.back();
			pluginList_.pop_back();
			pBase->Destroy();
			delete pBase;
		}

		if(vm_->IsInitilized())
			ret = vm_->Exec(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

		GetSyscalls()->Print("Recordsystem shutingdown...\n");
		vm_->~Q3Vm();

		return ret;
		break;

	case GAME_CLIENT_CONNECT: // return functions!
	case BOTAI_START_FRAME:
		EXECUTE_EVENT_RETURN_ARG(command, EXECUTE_TYPE_BEFORE, ret, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		if(ret != 0)
			return ret;
		break;

	case GAME_CLIENT_THINK: // void functions!
	case GAME_CLIENT_USERINFO_CHANGED:
	case GAME_CLIENT_DISCONNECT:
	case GAME_CLIENT_BEGIN:
	case GAME_CLIENT_COMMAND:
	case GAME_CONSOLE_COMMAND:
		EXECUTE_EVENT_VOID_ARG12(command, EXECUTE_TYPE_BEFORE, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)

		if(command == GAME_CLIENT_COMMAND && eventItem && eventItem->GetHandled())
			return 0;
		break;
	}



	// ***************************************************
	// Forward to quake3 vm (defrag)
	// ***************************************************
	if(vm_->IsInitilized()) {
		ret = vm_->Exec(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
		if(command == GAME_CLIENT_CONNECT)
			ret = (int)vm_->ExplicitArgPtr(ret);
	}

	if(command == GAME_INIT) {
		// initialize clients
		for(int i = 0; i<MAX_CLIENTS; i++) {
			userList_.push_back(new Q3User(i));
			userList_[i]->Reset();
		}
	}


	// ***************************************************
	// Only for callbacks/hooks (after forward to vm)
	// ***************************************************
	switch(command) {
	case GAME_CLIENT_CONNECT: // return functions!
	case BOTAI_START_FRAME:
		EXECUTE_EVENT_RETURN_ARG(command, EXECUTE_TYPE_BEFORE, ret, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		break;

	case GAME_INIT: // void functions!
	case GAME_RUN_FRAME:
	case GAME_CLIENT_THINK:
	case GAME_CLIENT_USERINFO_CHANGED:
	case GAME_CLIENT_DISCONNECT:
	case GAME_CLIENT_BEGIN:
	case GAME_CLIENT_COMMAND:
	case GAME_CONSOLE_COMMAND:
		EXECUTE_EVENT_VOID_ARG12(command, EXECUTE_TYPE_AFTER, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		break;
	}

	return ret;
}

void Recordsystem::RegisterCvar(vmCvar_t *cvarPtr, const char name[MAX_CVAR_VALUE_STRING], 
								const char defaultValue[MAX_CVAR_VALUE_STRING], int flags, qboolean track) {

	VmCvarItem *item = new VmCvarItem(cvarPtr, name, defaultValue, flags, track);
	GetSyscalls()->CvarRegister( item->vmCvar, item->cvarName, item->defaultString, item->cvarFlags );
	if ( item->vmCvar )
		item->modificationCount = item->vmCvar->modificationCount;

	cvarList_.push_back(item);
}

int Recordsystem::GetUserCount() {
	return MAX_CLIENTS;
}

Q3User *Recordsystem::GetUser(int playernum) {
	return userList_[playernum];
}


// **********************************************************************************************************
// *** PRIVATE **********************************************************************************************
// **********************************************************************************************************
playerState_t *Recordsystem::GetPlayerstate(int playernum) {
	return (playerState_t *) ((byte *) gameClients_ + gameClientSize_ * (playernum));
}

bool Recordsystem::GameInit(int levelTime, int randomSeed, int restart) {
	PluginBase *pBase;
	int i;

	GetSyscalls()->Print("------- Recordsystem initilizing -------\n");

	for(i = 0; i<gPluginStore.GetCount(); i++) {
		GetSyscalls()->Print(va("Init Plugin %i\n", i));
		pBase = gPluginStore.GetAt(i)->Create();
		pBase->Init();
		pluginList_.push_back(pBase);
	}

	RegisterCvar(&rs_api_server, "rs_api_server", "127.0.0.1", CVAR_ARCHIVE | CVAR_NORESTART, qfalse);
	RegisterCvar(&rs_api_port, "rs_api_port", "1234", CVAR_ARCHIVE | CVAR_NORESTART, qfalse);
	RegisterCvar(&rs_api_key, "rs_api_key", "-", CVAR_ARCHIVE | CVAR_NORESTART, qfalse);

	GetSyscalls()->Print(va("API-Server is %s:%i ...\n", rs_api_server.string, rs_api_port.integer));
	apiClient_ = new rpc::Client(rs_api_server.string, rs_api_port.integer);
	Q3dfApi_ = new Q3dfApi_Stub(apiClient_);

	GetSyscalls()->Print(va("Loading vm/qagame.qvm ...\n", rs_api_server.string, rs_api_port.integer));
	vm_ = new Q3Vm("vm/qagame.qvm", vm_syscall_);
	if(!vm_->IsInitilized()) {
		GetSyscalls()->Error("Faild initializing vm/qagame.qvm!\n");
		return false;
	}else
		GetSyscalls()->Print(va("Proxy to vm/qagame.qvm is initialized.\n", rs_api_server.string, rs_api_port.integer));

	GetSyscalls()->Print("Initialize users...\n");
	vm_syscall_->AddEventHandler(new Q3EventHandler(G_LOCATE_GAME_DATA, EXECUTE_TYPE_BEFORE, [](Q3EventArgs *e) {
		gRecordsystem->GetSyscalls()->Print("Locate game data from qvm...\n");
		gRecordsystem->SetGameData(e->GetParam(4), (playerState_t *)e->GetParamVMA(3), (gentity_t *)e->GetParamVMA(0), e->GetParam(2), e->GetParam(1));
	}));

	vm_syscall_->AddEventHandler(new Q3EventHandler(G_PRINT, EXECUTE_TYPE_BEFORE, [](Q3EventArgs *e) {
		PrintfRequest *pRequest = new PrintfRequest();
		pRequest->set_msg((const char *)e->GetParamVMA(0));
		//NullResponse *itemRes = new NullResponse();

		//EXECUTE_ASYNC(&Q3dfApi_Stub::Printf, gRecordsystem->GetQ3dfApi(), pRequest, itemRes, NULL);
	}));

	return true;
}

void Recordsystem::SetGameData(int clientSize, playerState_t *clients, gentity_t *gentities, int entitiesSize, int numEntities) {
	gameClientSize_ = clientSize;
	gameClients_ = clients;
	gentities_ = gentities;
	gentitySize_ = entitiesSize;
	numEntities_ = numEntities;
}

void Recordsystem::UpdateQuake3Cvars() {
	for (std::list<VmCvarItem *>::iterator it = cvarList_.begin(); it != cvarList_.end(); it++) {
		if((*it)->vmCvar) {
			GetSyscalls()->CvarUpdate( (*it)->vmCvar );

			if ( (*it)->modificationCount != (*it)->vmCvar->modificationCount ) {
				(*it)->modificationCount = (*it)->vmCvar->modificationCount;

				if ( (*it)->trackChange ) {
					GetSyscalls()->SendServerCommand(-1, va("print \"Server: %s changed to %s\n\"", (*it)->cvarName, (*it)->vmCvar->string));
				}
			}
		}
	}
}
