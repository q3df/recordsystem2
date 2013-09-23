#include "Recordsystem.h"
#include "Logger.h"
#include "Q3SysCallHook.h"
#include <utf8.h>
#include <functional>

using namespace std::placeholders;

extern Recordsystem *gRecordsystem;

#define EXECUTE_ASYNC(func, instance, sentMsg, replyMsg, callback)			\
	auto bind = std::bind((func),(instance),(sentMsg),(replyMsg));			\
	gRecordsystem->asyncExec_->ExecuteAsync((bind), (replyMsg), (callback));

void fix_utf8_string(std::string& str)
{
    std::string temp;
    utf8::replace_invalid(str.begin(), str.end(), back_inserter(temp));
    str = temp;
}

Recordsystem::Recordsystem(syscall_t syscall)
	: asyncExec_(new ApiAsyncExecuter()),
	  vm_syscall_(new Q3SysCall(syscall)),
	  syscall_(new Q3SysCall(syscall)) {

	printHook1_ = new Q3SysCallHook(G_PRINT, EXECUTE_TYPE_BEFORE, [](Q3SysCallHook *hook) {
		::service::EchoRequest *echoArgs = new ::service::EchoRequest();
		::service::NullResponse *echoReply = new ::service::NullResponse();

		std::string msg((const char *)hook->getParamPtr(0));
		fix_utf8_string(msg);
		echoArgs->set_msg(msg);

		EXECUTE_ASYNC(&service::EchoService_Stub::Echo, gRecordsystem->apiEchoService, echoArgs, echoReply, [](::google::protobuf::Message *msg, ::google::protobuf::rpc::Error *err) {
			if(!err->IsNil()) {
				printf("echoStub.Echo.Error: %s\n", err->String().c_str());
			}else
				printf("echoStub.Echo.Return: %s\n", ((::service::NullResponse *)msg)->msg().c_str());
		});
	});

	vm_syscall_->addHook(printHook1_);

	apiClient = new ::google::protobuf::rpc::Client("127.0.0.1", 1234);
	apiEchoService = new ::service::EchoService_Stub(apiClient);
}

Recordsystem::~Recordsystem() {
	printf("Recordsystem::~Recordsystem();\r\n");
	apiClient->Close();
	delete printHook1_;
	delete syscall_;
	delete vm_syscall_;
}

int Recordsystem::VmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	int ret = 0;

	switch(command) {
	case GAME_INIT:
		syscall_->Printf("[Q3df] Recordsystem initilizing...\n");

		vm_ = new Q3Vm("vm/qagame.qvm", vm_syscall_);
		if(!vm_->IsInitilized()) {
			syscall_->Error("[Q3df:error] Faild initializing vm/qagame.qvm!\n");
			return 0;
		}
		break;

	case GAME_RUN_FRAME:
		asyncExec_->DoMainThreadWork();
		break;

	case GAME_SHUTDOWN:
		if(vm_->IsInitilized())
			ret = vm_->Exec(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

		syscall_->Printf("[Q3df] Recordsystem shutingdown...\n");
		vm_->~Q3Vm();
		
		return ret;
	}


	if(vm_->IsInitilized())
		ret = vm_->Exec(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

	if(command == GAME_CLIENT_CONNECT)
		return (int)vm_->ExplicitArgPtr(ret);
	else
		return ret;
}
