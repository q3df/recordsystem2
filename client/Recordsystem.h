#ifndef CLIENT_RECORDSYSTEM_H_
#define CLIENT_RECORDSYSTEM_H_

#include "Quake3.h"
#include "Q3SysCall.h"
#include "Q3Vm.h"

class Recordsystem {
public:
	Recordsystem(syscall_t syscall);
	~Recordsystem();

	int VmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);


private:
	Q3SysCall *syscall_;
	Q3Vm *vm_;
};

#endif CLIENT_RECORDSYSTEM_H_
