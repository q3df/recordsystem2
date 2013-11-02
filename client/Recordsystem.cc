#include "Recordsystem.h"
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

#define EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	{																														\
		HookHandlers::iterator it;																							\
		for (it = hookHandlers_.begin(); it != hookHandlers_.end(); it++) {													\
			if( (*it).first->eventType_ == (hookType) && (*it).first->executeType_ == (executeType)) {						\
				(*it).first->reset();																						\
				(*it).first->setParam(0, (arg1));																			\
				(*it).first->setParam(1, (arg2));																			\
				(*it).first->setParam(2, (arg3));																			\
				(*it).first->setParam(3, (arg4));																			\
				(*it).first->setParam(4, (arg5));																			\
				(*it).first->setParam(5, (arg6));																			\
				(*it).first->setParam(6, (arg7));																			\
				(*it).first->setParam(7, (arg8));																			\
				(*it).first->setParam(8, (arg9));																			\
				(*it).first->setParam(9, (arg10));																			\
				(*it).first->setParam(10, (arg11));																			\
				(*it).first->setParam(11, (arg12));																			\
				(*it).first->executeCallback();																				\
				hook = (*it).first;																							\
			}																												\
		}																													\
	}

#define EXECUTE_CALLBACK_VOID_ARG1(hookType, executeType, arg1) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG2(hookType, executeType, arg1, arg2) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG3(hookType, executeType, arg1, arg2, arg3) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG4(hookType, executeType, arg1, arg2, arg3, arg4) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG5(hookType, executeType, arg1, arg2, arg3, arg4, arg5) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG6(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG7(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG8(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG9(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG10(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG11(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, 0)

#define EXECUTE_CALLBACK_VOID_ARG12(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)


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

	/*printHook1_ = new Q3SysCallHook(G_PRINT, EXECUTE_TYPE_BEFORE, [](Q3SysCallHook *hook) {
		EchoRequest *echoArgs = new EchoRequest();
		NullResponse *echoReply = new NullResponse();

		std::string msg((const char *)hook->getParamPtr(0));
		fix_utf8_string(msg);
		echoArgs->set_msg(msg);

		EXECUTE_ASYNC(&EchoService_Stub::Echo, gRecordsystem->apiEchoService, echoArgs, echoReply, [](Message *msg, rpc::Error *err) {
			if(!err->IsNil()) {
				printf("echoStub.Echo.Error: %s\n", err->String().c_str());
			}else if(!((NullResponse *)msg)->msg().empty())
				printf("echoStub.Echo.Return: %s\n", ((NullResponse *)msg)->msg().c_str());
		});
	});

	vm_syscall_->addHook(printHook1_);*/
}

Recordsystem::~Recordsystem() {
	apiClient_->Close();
	delete asyncExec_;
	delete Q3dfApi_;
	delete syscall_;
	delete vm_syscall_;
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

void Recordsystem::addHook(Q3SysCallHook *hook) {
	hookHandlers_.insert(std::pair<Q3SysCallHook*, Q3SysCallHook*>(hook, hook));
}

void Recordsystem::removeHook(Q3SysCallHook *hook) {
	HookHandlers::iterator it = hookHandlers_.find(hook);
	if( it != hookHandlers_.end())
		hookHandlers_.erase(it);
	else
		printf("ERROR REMOVE: it == hookHandlers_.end()\n");
}

int Recordsystem::VmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	int ret = 0;
	Q3SysCallHook *hook = NULL;
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

		EXECUTE_CALLBACK_VOID_ARG1(GAME_INIT, EXECUTE_TYPE_BEFORE, 0)
		break;

	case GAME_RUN_FRAME:
		UpdateQuake3Cvars();
		EXECUTE_CALLBACK_VOID_ARG1(GAME_RUN_FRAME, EXECUTE_TYPE_BEFORE, 0)
		GetAsyncExecuter()->DoMainThreadWork();
		break;

	case GAME_SHUTDOWN:
		EXECUTE_CALLBACK_VOID_ARG1(GAME_SHUTDOWN, EXECUTE_TYPE_BEFORE, 0)

		if(vm_->IsInitilized())
			ret = vm_->Exec(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

		GetSyscalls()->Printf("[Q3df] Recordsystem shutingdown...\n");
		vm_->~Q3Vm();

		return ret;
		break;

	case GAME_CLIENT_CONNECT: // return functions!
	case BOTAI_START_FRAME:
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

	if(vm_->IsInitilized())
		ret = vm_->Exec(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

	if(command == GAME_CLIENT_CONNECT)
		return (int)vm_->ExplicitArgPtr(ret);
	else
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
