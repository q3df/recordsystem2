#ifndef CLIENT_Q3VM_H_
#define CLIENT_Q3VM_H_

#include <stdio.h>

class Q3Vm {
public:
  Q3Vm(const char* path, byte* oldmem);
  ~Q3Vm();

  void Run();
  int QDECL Exec(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);
  qboolean Restart(qboolean savemem);
  void *ExplicitArgPtr( int intValue );
  int QDECL SysCalls(byte *memoryBase, int cmd, int *args);

private:
  vm_t *vm_;
  int stacksize_ = 0;
  bool initialized_ = false;
};

#endif // CLIENT_Q3VM_H_
