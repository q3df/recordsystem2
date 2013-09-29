#ifndef CLIENT_RECORDSYSTEM_H_
#define CLIENT_RECORDSYSTEM_H_

#include "Quake3.h"
#include "Q3SysCall.h"
#include "Q3Vm.h"
#include "ApiAsyncExecuter.h"
#include "Q3SysCallHook.h"
#include <utf8.h>
#include <functional>

#include <Q3dfApi.pb.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

using namespace google::protobuf;
using namespace service;

extern vmCvar_t rs_api_server;
extern vmCvar_t rs_api_port;
extern vmCvar_t rs_api_key;

#define EXECUTE_ASYNC(func, instance, sentMsg, replyMsg, callback)						\
	auto bind = std::bind((func),(instance),(sentMsg),(replyMsg));						\
	gRecordsystem->GetAsyncExecuter()->ExecuteAsync((bind), (replyMsg), (sentMsg), (callback));

void fix_utf8_string(std::string& str);
char *va( char *format, ... );

class Recordsystem {
public:
	Recordsystem(syscall_t syscall);
	~Recordsystem();

	ApiAsyncExecuter *GetAsyncExecuter();
	Q3dfApi *GetQ3dfApi();
	Q3SysCall *GetSyscalls();

	void addHook(Q3SysCallHook *hook);
	void removeHook(Q3SysCallHook *hook);
	
	int VmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);

private:
	void RegisterQuake3Cvars();
	void UpdateQuake3Cvars();

	ApiAsyncExecuter *asyncExec_;
	Q3SysCall *vm_syscall_;
	Q3SysCall *syscall_;
	Q3Vm *vm_;
	rpc::Client *apiClient_;
	Q3dfApi_Stub *Q3dfApi_;
	HookHandlers hookHandlers_;

};

extern Recordsystem *gRecordsystem;

#endif // CLIENT_RECORDSYSTEM_H_

