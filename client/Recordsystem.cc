#include "Recordsystem.h"
#include "Logger.h"

Recordsystem::Recordsystem(syscall_t syscall)
	: syscall_(new Q3SysCall(syscall)) {
}

Recordsystem::~Recordsystem() {
}

int Recordsystem::VmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	int ret = 0;

	switch(command) {
	case GAME_INIT:
		syscall_->Printf("[Q3df] Recordsystem initilizing...\n");

		vm_ = new Q3Vm("vm/qagame.qvm", syscall_);
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