#include "Recordsystem.h"
#include "Q3HookCallback.h"
#include <cstdarg>

using namespace google::protobuf;
using namespace service;

vmCvar_t rs_api_server;
vmCvar_t rs_api_port;
vmCvar_t rs_api_key;

typedef struct {
	vmCvar_t	*vmCvar;
	char		cvarName[256];
	char		defaultString[4096];
	int			cvarFlags;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
	qboolean	teamShader;         // track and if changed, update shader state
} cvarTable_t;

static cvarTable_t gameCvarTable[] = {
	{ &rs_api_server, "rs_api_server", "127.0.0.1", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse, qfalse },
	{ &rs_api_port, "rs_api_port", "1234", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse, qfalse },
	{ &rs_api_key, "rs_api_key", "-", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse, qfalse }
};

static int gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[0] );

void fix_utf8_string(std::string& str) {
    std::string temp;
    utf8::replace_invalid(str.begin(), str.end(), back_inserter(temp));
    str = temp;
}

char *va( const char *format, ... ) {
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

Recordsystem::Recordsystem(syscall_t syscall)
	: asyncExec_(new ApiAsyncExecuter()),
	  vm_syscall_(new Q3SysCall(syscall)),
	  syscall_(new Q3SysCall(syscall)) {


	testHook1_ = new Q3Hook(GAME_CLIENT_CONNECT, EXECUTE_TYPE_BEFORE, [](Q3Hook *hook) {
		gRecordsystem->GetSyscalls()->Printf(va("CL: %i | firstTime: %i | isBot: %i\r\n", hook->getParam(0), hook->getParam(1), hook->getParam(2)));
		hook->setReturnVMA((void *)"blabal bad hook user");
	});

	this->addHook(testHook1_);
}

Recordsystem::~Recordsystem() {
	apiClient_->Close();
	delete asyncExec_;
	delete Q3dfApi_;
	delete syscall_;
	delete vm_syscall_;
	delete testHook1_;
	hookHandlers_.clear();
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

void Recordsystem::addHook(Q3Hook *hook) {
	hookHandlers_.insert(std::pair<Q3Hook*, Q3Hook*>(hook, hook));
}

void Recordsystem::removeHook(Q3Hook *hook) {
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

		RegisterQuake3Cvars();

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


// *** PRIVATE **********************************************************************************************
void Recordsystem::RegisterQuake3Cvars() {
	int i;
	cvarTable_t	*cv;

	for ( i = 0, cv = gameCvarTable; i < gameCvarTableSize ; i++, cv++ ) {
		GetSyscalls()->CvarRegister( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
		if ( cv->vmCvar )
			cv->modificationCount = cv->vmCvar->modificationCount;
	}
}

void Recordsystem::UpdateQuake3Cvars() {
	int	i;
	cvarTable_t	*cv;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {
			GetSyscalls()->CvarUpdate( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) {
					GetSyscalls()->SendServerCommand(-1, va("print \"Server: %s changed to %s\n\"", cv->cvarName, cv->vmCvar->string));
				}
			}
		}
	}
}
