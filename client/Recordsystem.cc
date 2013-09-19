#include "Recordsystem.h"
#include "Logger.h"

#include "Q3SysCallHook.h"

extern Recordsystem *gRecordsystem;

Recordsystem::Recordsystem(syscall_t syscall)
	: vm_syscall_(new Q3SysCall(syscall)),
	  syscall_(new Q3SysCall(syscall)) {

	printHook1_ = new Q3SysCallHook(G_PRINT, EXECUTE_TYPE_BEFORE, [](Q3SysCallHook *hook) {
		gRecordsystem->syscall_->Printf("[Q3df] PRINT HOOK EXECUTED 1 BEFORE\n");
	});

	printHook2_ = new Q3SysCallHook(G_PRINT, EXECUTE_TYPE_AFTER,  [](Q3SysCallHook *hook) {
		gRecordsystem->syscall_->Printf("[Q3df] PRINT HOOK EXECUTED 2: AFTER\n\n");
	});

	vm_syscall_->addHook(printHook1_);
	vm_syscall_->addHook(printHook2_);
}

Recordsystem::~Recordsystem() {
	//vm_syscall_->removeHook(printHook_);
	delete printHook1_;
	delete printHook2_;
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
