/* VM_Run, VM_Exec, VM_Create, VM_Destroy, and VM_Restart
* originally from Q3Fusion (http://www.sourceforge.net/projects/q3fusion/)
*/

#define byteswap int_byteswap

int int_byteswap(int i) {
  byte b1,b2,b3,b4;

  b1 = i&255;
  b2 = (i>>8)&255;
  b3 = (i>>16)&255;
  b4 = (i>>24)&255;

  return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}


short short_byteswap(short s) {
  byte b1,b2;

  b1 = s&255;
  b2 = (s>>8)&255;

  return ((int)b1<<8) + b2;
}

//load the .qvm into the vm_t
//---
//this function opens the .qvm in a file stream, stores in dynamic mem
//reads header info, and loads vm into memory
//----
//vm = pointer to vm_t to load into
//path = filename to load
//oldmem = location to use for VM memory (default NULL)
Q3Vm::Q3Vm(const char* path, byte* oldmem) {
  if( !vm || !path || !path[0] ) return;
  memset(vm_, 0, sizeof(vm_t));

  vmHeader_t* header;
  byte* vmBase;
  int n;
  byte* src;
  int* lsrc;
  int* dst;
  vmOps_t op;
  int codeSegmentSize;
  vm_->swapped = qfalse;

  fileHandle_t fvm;
  //open VM file (use engine calls so we can easily read into .pk3)
  vm_->fileSize = g_syscall(G_FS_FOPEN_FILE, path, &fvm, FS_READ);
  //allocate memory block the size of the file
  vmBase = (byte*)malloc(vm_->fileSize);

  //malloc failed
  if (!vmBase) {
    memset(vm_, 0, sizeof(vm_t));
    return;
  }

  //read VM file into memory block
  g_syscall(G_FS_READ, vmBase, vm_->fileSize, fvm);
  g_syscall(G_FS_FCLOSE_FILE, fvm);

  header = (vmHeader_t*)vmBase;

  //if we are a big-endian machine, need to swap everything around
  if (header->vmMagic == VM_MAGIC_BIG) {
    rs_printf("WARNING: VM_Create: Big-endian magic number detected, will byteswap during load.\n");
    vm_->swapped = qtrue;
    header->vmMagic = byteswap(header->vmMagic);
    header->instructionCount = byteswap(header->instructionCount);
    header->codeOffset = byteswap(header->codeOffset);
    header->codeLength = byteswap(header->codeLength);
    header->dataOffset = byteswap(header->dataOffset);
    header->dataLength = byteswap(header->dataLength);
    header->litLength = byteswap(header->litLength);
    header->bssLength = byteswap(header->bssLength);
  }
  vm_->header = *header; //save header info in vm_t

  // check file
  if (header->vmMagic != VM_MAGIC || header->instructionCount <= 0 || header->codeLength <= 0) {
    free(vmBase);
    memset(vm_, 0, sizeof(vm_t));
    return;
  }

  // setup segments
  vm_->codeSegmentLen = header->instructionCount;
  vm_->dataSegmentLen = header->dataLength + header->litLength + header->bssLength;

  // calculate memory protection mask (including the stack?)
  for (vm_->dataSegmentMask = 1; ; vm_->dataSegmentMask <<= 1) {
    if(vm_->dataSegmentMask > vm_->dataSegmentLen + vm_stacksize) {
      vm_->dataSegmentMask--;
      break;
    }
  }

  //each opcode is 2 ints long, calculate total size of opcodes
  codeSegmentSize = vm->codeSegmentLen * sizeof(int) * 2;

  vm_->memorySize = codeSegmentSize + vm_->dataSegmentLen + vm_stacksize;
  //load memory code block (freed in VM_Destroy)
  //if we are reloading, we should keep the same memory location, otherwise, make more
  vm_->memory = (oldmem ? oldmem : (byte*)malloc(vm_->memorySize));
  //malloc failed
  if (!vm_->memory) {
    rs_printf("Unable to allocate VM memory chunk (size=%i)\n", vm->memorySize);
    free(vmBase);
    memset(vm_, 0, sizeof(vm_t));
    return;
  }
  //clear the memory
  memset(vm_->memory, 0, vm_->memorySize);

  // set pointers
  vm_->codeSegment = (int*)vm_->memory;
  vm_->dataSegment = (byte*)(vm_->memory + codeSegmentSize);
  vm_->stackSegment = (byte*)(vm_->dataSegment + vm_->dataSegmentLen);

  //setup registers
  vm_->opPointer = NULL;
  vm_->opStack = (int*)(vm_->stackSegment + stacksize_);
  vm_->opBase = vm_->dataSegmentLen + stacksize_ / 2;

  //load instructions from file to memory
  src = vmBase + header->codeOffset;
  dst = vm_->codeSegment;

  //loop through each instruction
  for (n = 0; n < header->instructionCount; n++) {
    //get its opcode and move src to the parameter field
    op = (vmOps_t)*src++;
    //write opcode (as int) and move dst to next int
    *dst++ = (int)op;

    switch( op ) {
    //these ops all have full 4-byte 'param's, which may need to be byteswapped
    //remaining args are drawn from stack
    case OP_ENTER:
    case OP_LEAVE:
    case OP_CONST:
    case OP_LOCAL:
    case OP_EQ:
    case OP_NE:
    case OP_LTI:
    case OP_LEI:
    case OP_GTI:
    case OP_GEI:
    case OP_LTU:
    case OP_LEU:
    case OP_GTU:
    case OP_GEU:
    case OP_EQF:
    case OP_NEF:
    case OP_LTF:
    case OP_LEF:
    case OP_GTF:
    case OP_GEF:
    case OP_BLOCK_COPY:
      *dst = *(int*)src;
      if (vm->swapped == qtrue)
        *dst = byteswap(*dst);
      dst++;
      src += 4;
      break;
    //this op has only a single byte 'param' (draws 1 arg from stack)
    case OP_ARG:
      *dst++ = (int)*src++;
      break;
    //remaining ops require no 'param' (draw all, if any, args from stack)
    default:
      *dst++ = 0;
      break;
    }
  }


  // load data segment from file to memory
  lsrc = (int*)(vmBase + header->dataOffset);
  dst = (int*)(vm->dataSegment);

  //loop through each 4-byte data block (even though data may be single bytes)
  for (n = 0; n < header->dataLength/sizeof(int); n++) {
    *dst = *lsrc++;
    //swap if need-be
    if (vm->swapped == qtrue)
      *dst = byteswap(*dst);
    dst++;
  }

  //copy remaining data into the lit segment
  memcpy(dst, lsrc, header->litLength);

  //free file from memory
  free(vmBase);

  //a winner is us
  return qtrue;
}

Q3Vm::~Q3Vm() {
  if (vm_->memory)
    free(vm_->memory);

  memset(vm_, 0, sizeof(vm_t));
}


void Q3Vm::Run() {
  vmOps_t   op;
  int   param;

  // local registers
  int   *opStack;
  int   *opPointer;

  // constants /not changed during execution/
  byte    *dataSegment;
  unsigned  dataSegmentMask;

  opStack = vm_->opStack;
  opPointer = vm_->opPointer;

  dataSegment = vm_->dataSegment;
  dataSegmentMask = vm_->dataSegmentMask;

  //keep going until opPointer is NULL
  //opPointer is set in OP_LEAVE, stored in the function stack
  //VM_Exec sets this to NULL before calling so that as soon as vmMain is done, execution stops
  do {
    //fetch opcode
    op = (vmOps_t)opPointer[0];
    //get the param
    param = opPointer[1];
    //move to the next opcode
    opPointer += 2;

    //here's the magic
    switch( op ) {
//
// aux
//
    //undefined
    case OP_UNDEF:
    //no op?
    case OP_NOP:
    //break to debugger?
    case OP_BREAK:
    //anything else
    default:
      rs_printf("ERROR: VM_Run: Unhandled opcode(%i)", op);
      break;

//
// subroutines
//
//jumps to a specific opcode
#define GOTO(x) {opPointer = vm_->codeSegment + (x) * 2;}

    //enter a function, assign function parameters (length=param) from stack
    case OP_ENTER:
      vm_->opBase -= param;
      *((int*)(dataSegment + vm_->opBase) + 1) = *opStack++;
      break;

    //leave a function, move opcode pointer to previous function
    case OP_LEAVE:
      opPointer = vm_->codeSegment + *((int*)(dataSegment + vm_->opBase) + 1);
      vm_->opBase += param;
      break;

    //call a function at address stored in opStack[0]
    case OP_CALL:
      param = opStack[0];

      //CyberMind - param(opStack[0]) is the function address, negative means a engine trap
      //added fix for external function pointers
      //if param is greater than the memorySize, it's a real function pointer, so call it
      if( param < 0 || param >= vm_->memorySize) {
        int ret = 0, realfunc = 0;
        int* args = NULL;
        //int     *fp;

        // system trap or real system function

        // save local registers for recursive execution
        //vm_->opBase = opBase;
        vm_->opStack = opStack;
        vm_->opPointer = opPointer;

        //clear hook var
        vm_->hook_realfunc = 0;

        args = (int *)(dataSegment + vm_->opBase) + 2;

        //if a trap function, call our local syscall, which parses each message
        if (param < 0) {
          ret = vm__SysCalls( dataSegment, (-param - 1), args );
        //otherwise it's a real function call, grab args and call function
        } else {
          //cdecl calling convention says caller (us) cleans stack
          //so we can stuff the args without worry of stack corruption
          ret = ((pfn_t)param)(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11]);
        }

        // restore local registers
        //opBase = vm_->opBase;
        opStack = vm_->opStack;
        opPointer = vm_->opPointer;

        //if we are running a vm_ function due to hook
        //and we have a real vm_ func to call, call it
        if (vm_->hook_realfunc && param >= vm_->memorySize) {
          //replace func address with return address
          opStack[0] = (int)(opPointer - vm_->codeSegment);
          GOTO (vm_->hook_realfunc)
        //otherwise we use the syscall/hook func return value
        } else {
          opStack[0] = ret;
        }
        break;
      }
      //replace func address with return address
      opStack[0] = (int)(opPointer - vm_->codeSegment); // push pc /return address/
      //jump to vm_ function at address
      GOTO( param )
      break;

//
// stack
//
    //pushes a 0 onto the end of the stack
    case OP_PUSH : opStack--; opStack[0] = 0;       break;
    //pops the last value off the end of the stack
    case OP_POP  : opStack++;           break;
    //pushes a specified value onto the end of the stack
    case OP_CONST: opStack--; opStack[0] = param;     break;
    //pushes a specified
    case OP_LOCAL: opStack--; opStack[0] = param + vm_->opBase;  break;


//
// branching
//
#define SOP(operation) {if (opStack[1] operation opStack[0]) GOTO(param); opStack += 2;}
#define UOP(operation) {if (*(unsigned int*)&opStack[1] operation *(unsigned int*)&opStack[0]) GOTO(param); opStack += 2;}
#define FOP(operation) {if (*(float*)&opStack[1] operation *(float*)&opStack[0]) GOTO(param); opStack += 2;}

    //jump to address in opStack[0], and pop
    case OP_JUMP: GOTO(*opStack++)  break;
    //if opStack[1] == opStack[0], goto address in param
    case OP_EQ  : SOP(==) break;
    //if opStack[1] != opStack[0], goto address in param
    case OP_NE  : SOP(!=) break;
    //if opStack[1] < opStack[0], goto address in param
    case OP_LTI : SOP(< ) break;
    //if opStack[1] <= opStack[0], goto address in param
    case OP_LEI : SOP(<=) break;
    //if opStack[1] > opStack[0], goto address in param
    case OP_GTI : SOP(> ) break;
    //if opStack[1] >= opStack[0], goto address in param
    case OP_GEI : SOP(>=) break;
    //if opStack[1] < opStack[0], goto address in param (unsigned)
    case OP_LTU : UOP(< ) break;
    //if opStack[1] <= opStack[0], goto address in param (unsigned)
    case OP_LEU : UOP(<=) break;
    //if opStack[1] > opStack[0], goto address in param (unsigned)
    case OP_GTU : UOP(> ) break;
    //if opStack[1] >= opStack[0], goto address in param (unsigned)
    case OP_GEU : UOP(>=) break;
    //if opStack[1] == opStack[0], goto address in param (float)
    case OP_EQF : FOP(==) break;
    //if opStack[1] != opStack[0], goto address in param (float)
    case OP_NEF : FOP(!=) break;
    //if opStack[1] < opStack[0], goto address in param (float)
    case OP_LTF : FOP(< ) break;
    //if opStack[1] <= opStack[0], goto address in param (float)
    case OP_LEF : FOP(<=) break;
    //if opStack[1] > opStack[0], goto address in param (float)
    case OP_GTF : FOP(> ) break;
    //if opStack[1] >= opStack[0], goto address in param (float)
    case OP_GEF : FOP(>=) break;

//
// memory I/O: masks protect main memory
//

    //get value at address stored in opStack[0], store in opStack[0]
    //(do neccesary conversions)
    //this is essentially the 'dereferencing' opcode set
    //1-byte
    case OP_LOAD1:
      if (opStack[0] >= vm_->memorySize)
        opStack[0] = *(byte*)(opStack[0]);
      else
        opStack[0] = dataSegment[opStack[0] & dataSegmentMask];

      break;

    //2-byte
    case OP_LOAD2:
      if (opStack[0] >= vm_->memorySize)
        opStack[0] = *(unsigned short*)(opStack[0]);
      else
        opStack[0] = *(unsigned short*)&dataSegment[opStack[0] & dataSegmentMask];

      break;

    //4-byte
    case OP_LOAD4:
      if (opStack[0] >= vm_->memorySize)
        opStack[0] = *(int*)(opStack[0]);
      else
        opStack[0] = *(int*)&dataSegment[opStack[0] & dataSegmentMask];

      break;

    //store a value from opStack[0] into address stored in opStack[1]
    //1-byte
    case OP_STORE1:
      if (opStack[1] >= vm_->memorySize)
        *(byte*)(opStack[1]) = (byte)(opStack[0] & 0xFF  );
      else
        dataSegment[opStack[1] & dataSegmentMask] = (byte)(opStack[0] & 0xFF  );

      opStack += 2;
      break;
    //2-byte
    case OP_STORE2:
      if (opStack[1] >= vm_->memorySize)
        *(unsigned short*)(opStack[1]) = (unsigned short)(opStack[0] & 0xFFFF);
      else
        *(unsigned short*)&dataSegment[opStack[1] & dataSegmentMask] =  (unsigned short)(opStack[0] & 0xFFFF);

      opStack += 2;
      break;
    //4-byte
    case OP_STORE4:
      if (opStack[1] >= vm_->memorySize)
        *(int*)(opStack[1]) = opStack[0];
      else
        *(int*)&dataSegment[opStack[1] & dataSegmentMask] = opStack[0];

      opStack += 2;
      break;


    //set a function-call arg (offset = param) to the value in opStack[0]
    case OP_ARG   : *(int*)&dataSegment[(param + vm_->opBase) & dataSegmentMask] = opStack[0]; opStack++; break;

    //copy mem at address pointed to by opStack[0] to address pointed to by opStack[1]
    //for 'param' number of bytes
    case OP_BLOCK_COPY:
      {
        int* from = (int*)&dataSegment[opStack[0] & dataSegmentMask];
        int* to = (int*)&dataSegment[opStack[1] & dataSegmentMask];

        if( param & 3 ) {
          g_syscall(G_ERROR, "[RS] vm__Run: OP_BLOCK_COPY not dword aligned" );
        }

        // FIXME: assume pointers don't overlap?
        param >>= 2;
        do {
          *to++ = *from++;
        } while( --param );

        opStack += 2;
      }
      break;

//
// arithmetic and logic
//
#undef SOP
#undef UOP
#undef FOP
//signed arithmetic
#define SOP(operation)  {opStack[1] operation opStack[0]; opStack++;}
//unsigned arithmetic
#define UOP(operation)  {*(unsigned int*)&opStack[1] operation *(unsigned int*)&opStack[0]; opStack++;}
//floating point arithmetic
#define FOP(operation)  {*(float*)&opStack[1] operation *(float*)&opStack[0]; opStack++;}
//signed arithmetic (on self)
#define SSOP(operation) {opStack[0] =operation opStack[0];}
//floating point arithmetic (on self)
#define SFOP(operation) {*(float*)&opStack[0] =operation *(float*)&opStack[0];}

    //sign extensions
    case OP_SEX8 : if(opStack[0] & 0x80)  opStack[0] |= 0xFFFFFF00; break;
    case OP_SEX16: if(opStack[0] & 0x8000)  opStack[0] |= 0xFFFF0000; break;
    //make negative
    case OP_NEGI : SSOP( - ) break;
    //add opStack[0] to opStack[1], store in opStack[1]
    case OP_ADD  : SOP( += ) break;
    //subtract opStack[0] from opStack[1], store in opStack[1]
    case OP_SUB  : SOP( -= ) break;
    //divide opStack[0] into opStack[1], store in opStack[1]
    case OP_DIVI : SOP( /= ) break;
    //divide opStack[0] into opStack[1], store in opStack[1] (unsigned)
    case OP_DIVU : UOP( /= ) break;
    //modulus opStack[0] into opStack[1], store in opStack[1]
    case OP_MODI : SOP( %= ) break;
    //modulus opStack[0] into opStack[1], store in opStack[1] (unsigned)
    case OP_MODU : UOP( %= ) break;
    //multiply opStack[0] and opStack[1], store in opStack[1]
    case OP_MULI : SOP( *= ) break;
    //multiply opStack[0] and opStack[1], store in opStack[1] (unsigned)
    case OP_MULU : UOP( *= ) break;
    //bitwise AND opStack[0] and opStack[1], store in opStack[1]
    case OP_BAND : SOP( &= ) break;
    //bitwise OR opStack[0] and opStack[1], store in opStack[1]
    case OP_BOR  : SOP( |= ) break;
    //bitwise XOR opStack[0] and opStack[1], store in opStack[1]
    case OP_BXOR : SOP( ^= ) break;
    //bitwise one's compliment opStack[0], store in opStack[1]
    case OP_BCOM : SSOP( ~ ) break;
    //bitwise LEFTSHIFT opStack[1] by opStack[0] bits, store in opStack[1]
    case OP_LSH  : UOP( <<= ) break;
    //bitwise RIGHTSHIFT opStack[1] by opStack[0] bits, store in opStack[1]
    case OP_RSHI : SOP( >>= ) break;
    //bitwise RIGHTSHIFT opStack[1] by opStack[0] bits, store in opStack[1] (unsigned)
    case OP_RSHU : UOP( >>= ) break;
    //make negative (float)
    case OP_NEGF : SFOP( - ) break;
    //add opStack[0] to opStack[1], store in opStack[1] (float)
    case OP_ADDF : FOP( += ) break;
    //subtract opStack[0] from opStack[1], store in opStack[1] (float)
    case OP_SUBF : FOP( -= ) break;
    //divide opStack[0] into opStack[1], store in opStack[1] (float)
    case OP_DIVF : FOP( /= ) break;
    //multiply opStack[0] and opStack[1], store in opStack[1] (float)
    case OP_MULF : FOP( *= ) break;

//
// format conversion
//
    //convert opStack[0] int->float
    case OP_CVIF: *(float *)&opStack[0] = (float)opStack[0]; break;
    //convert opStack[0] float->int
    case OP_CVFI: opStack[0] = (int)(*(float *)&opStack[0]); break;
    }
  } while( opPointer );

//  vm_->opBase = opBase;
  vm_->opStack = opStack;
//  vm->opPointer = opPointer;
}

//public function to begin the process of executing a VM
//----
//stuff args into the VM stack
//begin VM with VM_Run
//---
//vm = pointer to to VM
//command = GAME instruction to run
//arg# = args to command
int Q3Vm::Exec(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
  int* args;

  // prepare local stack
  vm_->opBase -= 15 * sizeof( int );
  args = (int *)(vm_->dataSegment + vm_->opBase);

  // push all params
  args[ 0] = 0;
  args[ 1] = (int)(vm_->opPointer - vm_->codeSegment); // save pc
  args[ 2] = command;
  args[ 3] = arg0;
  args[ 4] = arg1;
  args[ 5] = arg2;
  args[ 6] = arg3;
  args[ 7] = arg4;
  args[ 8] = arg5;
  args[ 9] = arg6;
  args[10] = arg7;
  args[11] = arg8;
  args[12] = arg9;
  args[13] = arg10;
  args[14] = arg11;

  vm_->opPointer = NULL; //VM_Run stops execution when opPointer is NULL

  //(ready) move back in stack to save pc
  vm_->opStack--;
  vm_->opStack[0] = (vm->opPointer - vm_->codeSegment);
  //(set) move opPointer to start of opcodes
  vm_->opPointer = vm->codeSegment;

  //GO!
  Run();

  // restore previous state
  vm_->opPointer = vm_->codeSegment + args[1];
  vm_->opBase += 15 * sizeof( int );

  // pick return value from stack
  return *vm_->opStack++;
}

//modified to save old memory pointer and pass it to VM_Create
//so we can keep the same memory location (might be desirable at
//some point in the future, but for now, we don't even reload)

//keep in mind...SHIT WILL BREAK IF YOU SWAP QVMS AND THEN RESTART
//do NOT blame me for assuming the file will stay the same
qboolean Q3Vm::Restart(vm_t *vm, qboolean savemem) {
  if(!vm) return qfalse;

  char name[MAX_QPATH];
  byte* oldmem = NULL;

  //save filename (we need this to reload the same file, obviously)
  strncpy(name, vm->name, sizeof(name));

  //save memory pointer or free it
  if (savemem == qtrue)
    oldmem = vm->memory;
  else
    free(vm->memory);

  //kill it!
  memset(vm, 0, sizeof(vm_t));

  //reload
  if (!VM_Create(vm, name, oldmem)) {
    VM_Destroy(vm);
    return qfalse;
  }

  return qtrue;
}

void *Q3Vm::ExplicitArgPtr( vm_t *vm, int intValue ) {
  if ( !intValue ) {
    return NULL;
  }

  // currentVM is missing on reconnect here as well?
  if ( vm==NULL )
    return NULL;

  return (void *)(vm->dataSegment + (intValue & vm->dataSegmentMask));
}
