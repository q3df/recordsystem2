#ifndef CLIENT_Q3VM_H_
#define CLIENT_Q3VM_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Q3SysCall.h"
#include "Quake3.h"

//handling syscalls from QVM (passing them to engine)
//this adds the base VM address to a given value
#define add(x)    ((x) ? (void*)((x) + (int)memoryBase) : NULL)
//this subtracts the base VM address from a given value
#define sub(x)    ((x) ? (void*)((x) - (int)memoryBase) : NULL)
//this gets an argument value
#define arg(x)    (args[(x)])
//this adds the base VM address to an argument value
#define ptr(x)    (add(arg(x)))
#define	VMF(x)	((*(float*)&args[(x)]))

class Q3Vm {
public:
	Q3Vm(const char* path, Q3SysCall *syscall);
	~Q3Vm();

	void Run();
	int Exec(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);
	qboolean Restart(qboolean savemem);
	void *ExplicitArgPtr( int intValue );
	qboolean IsInitilized() { return initialized_; }

private:
	qboolean Create(const char* path, byte* oldmem);
	int SysCalls(byte *memoryBase, int cmd, int *args);

	vm_t vm_;
	int stacksize_;
	qboolean initialized_;
	Q3SysCall *syscall_;
};

#endif // CLIENT_Q3VM_H_
