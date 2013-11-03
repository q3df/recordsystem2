#include "Recordsystem.h"
#include "Q3HookCallback.h"
#include <cstdarg>

using namespace google::protobuf;
using namespace service;
using namespace std::placeholders;

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
	apiClient_->Close();
	delete asyncExec_;
	delete Q3dfApi_;
	delete syscall_;
	delete vm_syscall_;
	hookHandlers_.clear();
	cvarList.clear();
}

ApiAsyncExecuter *Recordsystem::GetAsyncExecuter() {
	return asyncExec_;
}

Q3dfApi *Recordsystem::GetQ3dfApi() {
	return Q3dfApi_;
}

Q3SysCall *Recordsystem::GetSyscalls() {
	return syscall_;
}

void Recordsystem::AddHook(Q3Hook *hook) {
	hookHandlers_.insert(std::pair<Q3Hook*, Q3Hook*>(hook, hook));
}

void Recordsystem::RemoveHook(Q3Hook *hook) {
	HookHandlers::iterator it = hookHandlers_.find(hook);
	if( it != hookHandlers_.end())
		hookHandlers_.erase(it);
}

int Recordsystem::VmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	int ret = 0;
	Q3Hook *hook = NULL;
	if(hook == NULL)
		hook = NULL; // fixed compile error because of unused...

	switch(command) {
	case GAME_INIT:
		GetSyscalls()->Printf("------- Recordsystem initilizing -------\n");

		RegisterCvar(&rs_api_server, "rs_api_server", "127.0.0.1", CVAR_ARCHIVE | CVAR_NORESTART, qfalse);
		RegisterCvar(&rs_api_port, "rs_api_port", "1234", CVAR_ARCHIVE | CVAR_NORESTART, qfalse);
		RegisterCvar(&rs_api_key, "rs_api_key", "-", CVAR_ARCHIVE | CVAR_NORESTART, qfalse);

		GetSyscalls()->Printf(va("[Q3df] API-Server is %s:%i ...\n", rs_api_server.string, rs_api_port.integer));
		apiClient_ = new rpc::Client(rs_api_server.string, rs_api_port.integer);
		Q3dfApi_ = new Q3dfApi_Stub(apiClient_);

		GetSyscalls()->Printf(va("[Q3df] Loading vm/qagame.qvm ...\n", rs_api_server.string, rs_api_port.integer));
		vm_ = new Q3Vm("vm/qagame.qvm", vm_syscall_);
		if(!vm_->IsInitilized()) {
			GetSyscalls()->Error("[Q3df:error] Faild initializing vm/qagame.qvm!\n");
			return 0;
		}else
			GetSyscalls()->Printf(va("[Q3df] Proxy to vm/qagame.qvm is initialized.\n", rs_api_server.string, rs_api_port.integer));

		EXECUTE_CALLBACK_VOID_ARG12(command, EXECUTE_TYPE_BEFORE, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		break;

	case GAME_RUN_FRAME:
		UpdateQuake3Cvars();
		EXECUTE_CALLBACK_VOID_ARG12(command, EXECUTE_TYPE_BEFORE, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		GetAsyncExecuter()->DoMainThreadWork();
		break;

	case GAME_SHUTDOWN:
		EXECUTE_CALLBACK_VOID_ARG12(command, EXECUTE_TYPE_BEFORE, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)

		if(vm_->IsInitilized())
			ret = vm_->Exec(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

		GetSyscalls()->Printf("[Q3df] Recordsystem shutingdown...\n");
		vm_->~Q3Vm();

		return ret;
		break;

	case GAME_CLIENT_CONNECT: // return functions!
	case BOTAI_START_FRAME:
		EXECUTE_CALLBACK_RETURN_ARG(command, EXECUTE_TYPE_BEFORE, ret, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		if(ret != 0)
			return ret;
		break;

	case GAME_CLIENT_THINK: // void functions!
	case GAME_CLIENT_USERINFO_CHANGED:
	case GAME_CLIENT_DISCONNECT:
	case GAME_CLIENT_BEGIN:
	case GAME_CLIENT_COMMAND:
	case GAME_CONSOLE_COMMAND:
		EXECUTE_CALLBACK_VOID_ARG12(command, EXECUTE_TYPE_BEFORE, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		break;
	}

	if(vm_->IsInitilized()) {
		ret = vm_->Exec(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
		if(command == GAME_CLIENT_CONNECT)
			ret = (int)vm_->ExplicitArgPtr(ret);
	}

	switch(command) {
	case GAME_CLIENT_CONNECT: // return functions!
	case BOTAI_START_FRAME:
		EXECUTE_CALLBACK_RETURN_ARG(command, EXECUTE_TYPE_BEFORE, ret, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
		break;

	case GAME_INIT: // void functions!
	case GAME_RUN_FRAME:
	case GAME_CLIENT_THINK:
	case GAME_CLIENT_USERINFO_CHANGED:
	case GAME_CLIENT_DISCONNECT:
	case GAME_CLIENT_BEGIN:
	case GAME_CLIENT_COMMAND:
	case GAME_CONSOLE_COMMAND:
		EXECUTE_CALLBACK_VOID_ARG12(command, EXECUTE_TYPE_AFTER, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
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

	cvarList.push_back(item);
}

// *** PRIVATE **********************************************************************************************
void Recordsystem::UpdateQuake3Cvars() {
	for (std::list<VmCvarItem *>::iterator it = cvarList.begin(); it != cvarList.end(); it++) {
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
