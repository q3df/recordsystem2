/* VM_Run, VM_Exec, VM_Create, VM_Destroy, and VM_Restart
* originally from Q3Fusion (http://www.sourceforge.net/projects/q3fusion/)
*/
#include "Q3Vm.h"
#include "Logger.h"

extern "C" {
	void fix_utf8_string(std::string& str);
	const char *va( const char *format, ... );
}

int PASSFLOAT2(float x) {
	float	floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

#define byteswap int_byteswap
int int_byteswap(int i) {
	byte b1,b2,b3,b4;

	b1 = i&255;
	b2 = (i>>8)&255;
	b3 = (i>>16)&255;
	b4 = (i>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

short int_byteswap(short s) {
	byte b1,b2;

	b1 = s&255;
	b2 = (s>>8)&255;

	return ((int)b1<<8) + b2;
}


/**
*/
Q3Vm::Q3Vm(const char* path, Q3SysCall *syscall) 
  : syscall_(syscall) {
	if( !path || !path[0] ) return;
	memset(&vm_, 0, sizeof(vm_t));

	stacksize_ = 0;
	initialized_ = qfalse;
	initialized_ = this->Create(path, NULL);
}


/**
*/
Q3Vm::~Q3Vm() {
	if (vm_.memory)
		free(vm_.memory);

	memset(&vm_, 0, sizeof(vm_t));
}


/**
*/
void Q3Vm::Run() {
	vmOps_t   op;
	int   param;

	// local registers
	int   *opStack;
	int   *opPointer;

	// constants /not changed during execution/
	byte    *dataSegment;
	unsigned  dataSegmentMask;

	opStack = vm_.opStack;
	opPointer = vm_.opPointer;

	dataSegment = vm_.dataSegment;
	dataSegmentMask = vm_.dataSegmentMask;

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
			syscall_->PrintError(va("VM_Run: Unhandled opcode(%i)", op));
			break;

			//
			// subroutines
			//
			//jumps to a specific opcode
#define GOTO(x) {opPointer = vm_.codeSegment + (x) * 2;}

			//enter a function, assign function parameters (length=param) from stack
		case OP_ENTER:
			vm_.opBase -= param;
			*((int*)(dataSegment + vm_.opBase) + 1) = *opStack++;
			break;

			//leave a function, move opcode pointer to previous function
		case OP_LEAVE:
			opPointer = vm_.codeSegment + *((int*)(dataSegment + vm_.opBase) + 1);
			vm_.opBase += param;
			break;

			//call a function at address stored in opStack[0]
		case OP_CALL:
			param = opStack[0];

			//CyberMind - param(opStack[0]) is the function address, negative means a engine trap
			//added fix for external function pointers
			//if param is greater than the memorySize, it's a real function pointer, so call it
			if( param < 0 || param >= vm_.memorySize) {
				int ret = 0;
				int* args = NULL;
				//int     *fp;

				// system trap or real system function

				// save local registers for recursive execution
				//vm_.opBase = opBase;
				vm_.opStack = opStack;
				vm_.opPointer = opPointer;

				//clear hook var
				vm_.hook_realfunc = 0;

				args = (int *)(dataSegment + vm_.opBase) + 2;

				//if a trap function, call our local syscall, which parses each message
				if (param < 0) {
					ret = this->SysCalls( dataSegment, (-param - 1), args );
					//otherwise it's a real function call, grab args and call function
				} else {
					//cdecl calling convention says caller (us) cleans stack
					//so we can stuff the args without worry of stack corruption
					ret = ((pfn_t)param)(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11]);
				}

				// restore local registers
				//opBase = vm_.opBase;
				opStack = vm_.opStack;
				opPointer = vm_.opPointer;

				//if we are running a vm_ function due to hook
				//and we have a real vm_ func to call, call it
				if (vm_.hook_realfunc && param >= vm_.memorySize) {
					//replace func address with return address
					opStack[0] = (int)(opPointer - vm_.codeSegment);
					GOTO (vm_.hook_realfunc)
						//otherwise we use the syscall/hook func return value
				} else {
					opStack[0] = ret;
				}
				break;
			}
			//replace func address with return address
			opStack[0] = (int)(opPointer - vm_.codeSegment); // push pc /return address/
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
		case OP_LOCAL: opStack--; opStack[0] = param + vm_.opBase;  break;


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
			if (opStack[0] >= vm_.memorySize)
				opStack[0] = *(byte*)(opStack[0]);
			else
				opStack[0] = dataSegment[opStack[0] & dataSegmentMask];

			break;

			//2-byte
		case OP_LOAD2:
			if (opStack[0] >= vm_.memorySize)
				opStack[0] = *(unsigned short*)(opStack[0]);
			else
				opStack[0] = *(unsigned short*)&dataSegment[opStack[0] & dataSegmentMask];

			break;

			//4-byte
		case OP_LOAD4:
			if (opStack[0] >= vm_.memorySize)
				opStack[0] = *(int*)(opStack[0]);
			else
				opStack[0] = *(int*)&dataSegment[opStack[0] & dataSegmentMask];

			break;

			//store a value from opStack[0] into address stored in opStack[1]
			//1-byte
		case OP_STORE1:
			if (opStack[1] >= vm_.memorySize)
				*(byte*)(opStack[1]) = (byte)(opStack[0] & 0xFF  );
			else
				dataSegment[opStack[1] & dataSegmentMask] = (byte)(opStack[0] & 0xFF  );

			opStack += 2;
			break;
			//2-byte
		case OP_STORE2:
			if (opStack[1] >= vm_.memorySize)
				*(unsigned short*)(opStack[1]) = (unsigned short)(opStack[0] & 0xFFFF);
			else
				*(unsigned short*)&dataSegment[opStack[1] & dataSegmentMask] =  (unsigned short)(opStack[0] & 0xFFFF);

			opStack += 2;
			break;
			//4-byte
		case OP_STORE4:
			if (opStack[1] >= vm_.memorySize)
				*(int*)(opStack[1]) = opStack[0];
			else
				*(int*)&dataSegment[opStack[1] & dataSegmentMask] = opStack[0];

			opStack += 2;
			break;


			//set a function-call arg (offset = param) to the value in opStack[0]
		case OP_ARG   : *(int*)&dataSegment[(param + vm_.opBase) & dataSegmentMask] = opStack[0]; opStack++; break;

			//copy mem at address pointed to by opStack[0] to address pointed to by opStack[1]
			//for 'param' number of bytes
		case OP_BLOCK_COPY:
			{
				int* from = (int*)&dataSegment[opStack[0] & dataSegmentMask];
				int* to = (int*)&dataSegment[opStack[1] & dataSegmentMask];

				if( param & 3 ) {
					syscall_->Error("VM_Run: OP_BLOCK_COPY not dword aligned" );
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

	//  vm_.opBase = opBase;
	vm_.opStack = opStack;
	//  vm->opPointer = opPointer;
}


/**
*/
int Q3Vm::Exec(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	int* args;

	// prepare local stack
	vm_.opBase -= 15 * sizeof( int );
	args = (int *)(vm_.dataSegment + vm_.opBase);

	// push all params
	args[ 0] = 0;
	args[ 1] = (int)(vm_.opPointer - vm_.codeSegment); // save pc
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

	vm_.opPointer = NULL; //VM_Run stops execution when opPointer is NULL

	//(ready) move back in stack to save pc
	vm_.opStack--;
	vm_.opStack[0] = (vm_.opPointer - vm_.codeSegment);
	//(set) move opPointer to start of opcodes
	vm_.opPointer = vm_.codeSegment;

	//GO!
	Run();

	// restore previous state
	vm_.opPointer = vm_.codeSegment + args[1];
	vm_.opBase += 15 * sizeof( int );

	// pick return value from stack
	return *vm_.opStack++;
}


/**
*/
qboolean Q3Vm::Restart(qboolean savemem) {
	char name[MAX_QPATH];
	byte* oldmem = NULL;

	//save filename (we need this to reload the same file, obviously)
	strncpy(name, vm_.name, sizeof(name));

	//save memory pointer or free it
	if (savemem == qtrue)
		oldmem = vm_.memory;
	else
		free(vm_.memory);

	//kill it!
	memset(&vm_, 0, sizeof(vm_t));

	//reload
	if (!Create(name, oldmem)) {
		if (vm_.memory)
			free(vm_.memory);

		memset(&vm_, 0, sizeof(vm_t));
		initialized_ = qfalse;
		return qfalse;
	}

	initialized_ = qtrue;
	return qtrue;
}


/**
*/
void *Q3Vm::ExplicitArgPtr(int intValue) {
	if ( !intValue ) {
		return NULL;
	}

	// currentVM is missing on reconnect here as well?
	if ( vm_.dataSegment==NULL )
		return NULL;

	return (void *)(vm_.dataSegment + (intValue & vm_.dataSegmentMask));
}


/**
*/
qboolean Q3Vm::Create(const char *path, byte *oldmem) {
	vmHeader_t* header;
	byte* vmBase;
	int n;
	byte* src;
	int* lsrc;
	int* dst;
	vmOps_t op;
	int codeSegmentSize;
	vm_.swapped = qfalse;

	fileHandle_t fvm;
	//open VM file (use engine calls so we can easily read into .pk3)
	vm_.fileSize = syscall_->FSFOpenFile(path, &fvm, FS_READ);
	//allocate memory block the size of the file
	vmBase = (byte*)malloc(vm_.fileSize);

	//malloc failed
	if (!vmBase) {
		memset(&vm_, 0, sizeof(vm_t));
		return qfalse;
	}

	//read VM file into memory block
	syscall_->FSRead(vmBase, vm_.fileSize, fvm);
	syscall_->FSFCloseFile(fvm);

	header = (vmHeader_t*)vmBase;

	//if we are a big-endian machine, need to swap everything around
	if (header->vmMagic == VM_MAGIC_BIG) {
		syscall_->PrintWarning("VM_Create: Big-endian magic number detected, will byteswap during load.\n");
		vm_.swapped = qtrue;
		header->vmMagic = byteswap(header->vmMagic);
		header->instructionCount = byteswap(header->instructionCount);
		header->codeOffset = byteswap(header->codeOffset);
		header->codeLength = byteswap(header->codeLength);
		header->dataOffset = byteswap(header->dataOffset);
		header->dataLength = byteswap(header->dataLength);
		header->litLength = byteswap(header->litLength);
		header->bssLength = byteswap(header->bssLength);
	}
	vm_.header = *header; //save header info in vm_t

	// check file
	if (header->vmMagic != VM_MAGIC || header->instructionCount <= 0 || header->codeLength <= 0) {
		free(vmBase);
		memset(&vm_, 0, sizeof(vm_t));
		return qfalse;
	}

	// setup segments
	vm_.codeSegmentLen = header->instructionCount;
	vm_.dataSegmentLen = header->dataLength + header->litLength + header->bssLength;

	// calculate memory protection mask (including the stack?)
	for (vm_.dataSegmentMask = 1; ; vm_.dataSegmentMask <<= 1) {
		if(vm_.dataSegmentMask > vm_.dataSegmentLen + stacksize_) {
			vm_.dataSegmentMask--;
			break;
		}
	}

	//each opcode is 2 ints long, calculate total size of opcodes
	codeSegmentSize = vm_.codeSegmentLen * sizeof(int) * 2;

	vm_.memorySize = codeSegmentSize + vm_.dataSegmentLen + stacksize_;
	//load memory code block (freed in VM_Destroy)
	//if we are reloading, we should keep the same memory location, otherwise, make more
	vm_.memory = (oldmem ? oldmem : (byte*)malloc(vm_.memorySize));
	//malloc failed
	if (!vm_.memory) {
		syscall_->PrintError(va("Unable to allocate VM memory chunk (size=%i)\n", vm_.memorySize));
		free(vmBase);
		memset(&vm_, 0, sizeof(vm_t));
		return qfalse;
	}
	//clear the memory
	memset(vm_.memory, 0, vm_.memorySize);

	// set pointers
	vm_.codeSegment = (int*)vm_.memory;
	vm_.dataSegment = (byte*)(vm_.memory + codeSegmentSize);
	vm_.stackSegment = (byte*)(vm_.dataSegment + vm_.dataSegmentLen);

	//setup registers
	vm_.opPointer = NULL;
	vm_.opStack = (int*)(vm_.stackSegment + stacksize_);
	vm_.opBase = vm_.dataSegmentLen + stacksize_ / 2;

	//load instructions from file to memory
	src = vmBase + header->codeOffset;
	dst = vm_.codeSegment;

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
			if (vm_.swapped == qtrue)
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
	dst = (int*)(vm_.dataSegment);

	//loop through each 4-byte data block (even though data may be single bytes)
	for (n = 0; n < header->dataLength/(int)sizeof(int); n++) {
		*dst = *lsrc++;
		//swap if need-be
		if (vm_.swapped == qtrue)
			*dst = byteswap(*dst);
		dst++;
	}

	//copy remaining data into the lit segment
	memcpy(dst, lsrc, header->litLength);

	//free file from memory
	free(vmBase);

	return qtrue;
}


/**
*/
int Q3Vm::SysCalls(byte *memoryBase, int cmd, int *args) {
	//LOG(LOG_DEBUG, "%i", cmd);
	switch(cmd) {
	case G_PRINT: // ( const char *string );
		syscall_->Printf((const char *)ptr(0));
		return 0;
	case G_ERROR: // ( const char *string );
		syscall_->Error((const char *)ptr(0));
		return 0;
	case G_MILLISECONDS: // ( void );
		return syscall_->Milliseconds();
	case G_CVAR_REGISTER: // ( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
		syscall_->CvarRegister((vmCvar_t *)ptr(0), (const char *)ptr(1), (const char*)ptr(2), arg(3));
		return 0;
	case G_CVAR_UPDATE:	// ( vmCvar_t *vmCvar );
		syscall_->CvarUpdate((vmCvar_t *)ptr(0));
		return 0;
	case G_CVAR_SET: // ( const char *var_name, const char *value );
		syscall_->CvarSet((const char *)ptr(0), (const char *)ptr(1));
		return 0;
	case G_CVAR_VARIABLE_INTEGER_VALUE:	// ( const char *var_name );
		return syscall_->CvarVariableIntegerValue((const char *)ptr(0));
	case G_CVAR_VARIABLE_STRING_BUFFER:	// ( const char *var_name, char *buffer, int bufsize );
		syscall_->CvarVariableStringBuffer((const char *)ptr(0), (char *)ptr(1), arg(2));
		return 0;
	case G_ARGC: // ( void );
		return syscall_->Argc();
	case G_ARGV: // ( int n, char *buffer, int bufferLength );
		syscall_->Argv(arg(0), (char *)ptr(1), arg(2));
		return 0;
	case G_FS_FOPEN_FILE: // ( const char *qpath, fileHandle_t *file, fsMode_t mode );
		return syscall_->FSFOpenFile((const char *)ptr(0), (fileHandle_t *)ptr(1), (fsMode_t)arg(2));
	case G_FS_READ: // ( void *buffer, int len, fileHandle_t f );
		syscall_->FSRead(ptr(0), arg(1), arg(2));
		return 0;
	case G_FS_WRITE: // ( const void *buffer, int len, fileHandle_t f );
		syscall_->FSWrite(ptr(0), arg(1), arg(2));
		return 0;
	case G_FS_FCLOSE_FILE: // ( fileHandle_t f );
		syscall_->FSFCloseFile(arg(0));
		return 0;
	case G_SEND_CONSOLE_COMMAND: // ( int exec_when, const char *text )
		syscall_->SendConsoleCommand(arg(0), (const char *)ptr(1));
		return 0;
	case G_LOCATE_GAME_DATA: // ( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGameClient );
		syscall_->LocateGameData((gentity_t *)ptr(0), arg(1), arg(2), (playerState_t*)ptr(3), arg(4));
		return 0;
	case G_DROP_CLIENT: // ( int clientNum, const char *reason );
		syscall_->DropClient(arg(0), (const char *)ptr(1));
		return 0;
	case G_SEND_SERVER_COMMAND:	// ( int clientNum, const char *fmt );
		syscall_->SendServerCommand(arg(0), (const char *)ptr(1));
		return 0;
	case G_SET_CONFIGSTRING: // ( int num, const char *string );
		syscall_->SetConfigstring(arg(0), (const char *)ptr(1));
		return 0;
	case G_GET_CONFIGSTRING: // ( int num, char *buffer, int bufferSize );
		syscall_->GetConfigstring(arg(0), (char *)ptr(1), arg(2));
		return 0;
	case G_GET_USERINFO: // ( int num, char *buffer, int bufferSize );
		syscall_->GetUserinfo(arg(0), (char *)ptr(1), arg(2));
		return 0;
	case G_SET_USERINFO: // ( int num, const char *buffer );
		syscall_->SetUserinfo(arg(0), (const char *)ptr(1));
		return 0;
	case G_GET_SERVERINFO: // ( char *buffer, int bufferSize );
		syscall_->GetServerinfo((char *)ptr(0), arg(1));
		return 0;
	case G_SET_BRUSH_MODEL:	// ( gentity_t *ent, const char *name );
		syscall_->SetBrushModel((gentity_t *)ptr(0), (const char *)ptr(1));
		return 0;
	case G_TRACE: // ( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
		syscall_->Trace((trace_t *)ptr(0), (const vec_t *)ptr(1), (const vec_t *)ptr(2), (const vec_t *)ptr(3), (const vec_t *)ptr(4), arg(5), arg(6));
		return 0;
	case G_POINT_CONTENTS: // ( const vec3_t point, int passEntityNum );
		return syscall_->PointContents((const vec_t *)ptr(0), arg(1));
	case G_IN_PVS: // ( const vec3_t p1, const vec3_t p2 );
		return syscall_->InPVS((const vec_t *)ptr(0), (const vec_t *)ptr(1));
	case G_IN_PVS_IGNORE_PORTALS: // ( const vec3_t p1, const vec3_t p2 );
		return syscall_->InPVSIgnorePortals((const vec_t *)ptr(0), (const vec_t *)ptr(1));
	case G_ADJUST_AREA_PORTAL_STATE: // ( gentity_t *ent, qboolean open );
		syscall_->AdjustAreaPortalState((gentity_t *)ptr(0), (qboolean)arg(1));
		return 0;
	case G_AREAS_CONNECTED: // ( int area1, int area2 );
		return syscall_->AreasConnected(arg(0), arg(1));
	case G_LINKENTITY: // ( gentity_t *ent );
		syscall_->LinkEntity((gentity_t *)ptr(0));
		return 0;
	case G_UNLINKENTITY: // ( gentity_t *ent );
		syscall_->UnlinkEntity((gentity_t *)ptr(0));
		return 0;
	case G_ENTITIES_IN_BOX: // ( const vec3_t mins, const vec3_t maxs, gentity_t **list, int maxcount );
		return syscall_->EntitiesInBox((const vec_t *)ptr(0), (const vec_t *)ptr(1), (gentity_t **)ptr(2), arg(3));
	case G_ENTITY_CONTACT: // ( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
		return syscall_->EntityContact((const vec_t *)ptr(0), (const vec_t *)ptr(1), (const gentity_t *)ptr(2));
	case G_BOT_ALLOCATE_CLIENT:		// ( void );
		return syscall_->BotAllocateClient();
	case G_BOT_FREE_CLIENT:	// ( int clientNum );
		syscall_->BotFreeClient(arg(0));
		return 0;
	case G_GET_USERCMD: // ( int clientNum, usercmd_t *cmd )
		syscall_->GetUsercmd(arg(0), (usercmd_t *)ptr(1));
		return 0;
	case G_GET_ENTITY_TOKEN: // qboolean ( char *buffer, int bufferSize )
		return syscall_->GetEntityToken((char *)ptr(0), arg(1));
	case G_FS_GETFILELIST: // ( const char *path, const char *extension, char *listbuf, int bufsize ) {
		return syscall_->FSGetFileList((const char *)ptr(0), (const char *)ptr(1), (char *)ptr(2), arg(3));
	case G_DEBUG_POLYGON_CREATE: //(int color, int numPoints, vec3_t *points)
		return syscall_->DebugPolygonCreate(arg(0), arg(1), (vec3_t *)ptr(2));
	case G_DEBUG_POLYGON_DELETE: //(int id)
		syscall_->DebugPolygonDelete(arg(0));
		return 0;
	case G_REAL_TIME: //( qtime_t *qtime )
		return syscall_->RealTime((qtime_t *)ptr(0));
	case G_SNAPVECTOR: // (float *v)
		syscall_->SnapVector((float *)ptr(0));
		return 0;
	case G_TRACECAPSULE: // ( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
		syscall_->TraceCapsule((trace_t *)ptr(0), (const vec_t *)ptr(1), (const vec_t *)ptr(2), (const vec_t *)ptr(3), (const vec_t *)ptr(4), arg(5), arg(6));
		return 0;
	case G_ENTITY_CONTACTCAPSULE: // ( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
		return syscall_->EntityContactCapsule((const vec_t *)ptr(0), (const vec_t *)ptr(1), (const gentity_t *)ptr(2));
	case G_FS_SEEK: //( fileHandle_t f, long offset, int origin )
		return syscall_->FSSeek(arg(0), arg(1), arg(2));
	case BOTLIB_SETUP: //(void)
		return syscall_->BotLibSetup();
	case BOTLIB_SHUTDOWN: //(void)
		return syscall_->BotLibShutdown();
	case BOTLIB_LIBVAR_SET:	//(char *var_name, char *value)
		return syscall_->BotLibVarSet((char *)ptr(0), (char *)ptr(1));
	case BOTLIB_LIBVAR_GET:	//(char *var_name, char *value, int size)
		return syscall_->BotLibVarGet((char *)ptr(0), (char *)ptr(1), arg(2));
	case BOTLIB_PC_ADD_GLOBAL_DEFINE: //(char* string)
		return syscall_->BotLibDefine((char *)ptr(0));
	case BOTLIB_START_FRAME: //(float time)
		return syscall_->BotLibStartFrame(VMF(0));
	case BOTLIB_LOAD_MAP: //(const char *mapname)
		return syscall_->BotLibLoadMap((const char *)ptr(0));
	case BOTLIB_UPDATENTITY: //(int ent, void /* struct bot_updateentity_s */ *bue)
		return syscall_->BotLibUpdateEntity(arg(0), ptr(1));
	case BOTLIB_TEST: //(int parm0, char *parm1, vec3_t parm2, vec3_t parm3)
		return syscall_->BotLibTest(arg(0), (char *)ptr(1), (vec_t *)ptr(2), (vec_t *)ptr(3));
	case BOTLIB_GET_SNAPSHOT_ENTITY: //( int clientNum, int sequence )
		return syscall_->BotGetSnapshotEntity(arg(0), arg(1));
	case BOTLIB_GET_CONSOLE_MESSAGE: //(int clientNum, char *message, int size)
		return syscall_->BotGetServerCommand(arg(0), (char *)ptr(1), arg(2));
	case BOTLIB_USER_COMMAND: //(int clientNum, usercmd_t *ucmd)
		syscall_->BotUserCommand(arg(0), (usercmd_t *)ptr(1));
		return 0;
	case BOTLIB_AAS_ENTITY_INFO: //(int entnum, void /* struct aas_entityinfo_s */ *info)
		syscall_->AASEntityInfo(arg(0), ptr(1));
		return 0;
	case BOTLIB_AAS_INITIALIZED: //(void)
		return syscall_->AASInitialized();
	case BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX:	//(int presencetype, vec3_t mins, vec3_t maxs)
		syscall_->AASPresenceTypeBoundingBox(arg(0), (vec_t *)ptr(1), (vec_t *)ptr(2));
		return 0;
	case BOTLIB_AAS_TIME: //(void)
		return PASSFLOAT2(syscall_->AASTime());
	case BOTLIB_AAS_POINT_AREA_NUM:	//(vec3_t point)
		return syscall_->AASPointAreaNum((vec_t *)ptr(0));
	case BOTLIB_AAS_POINT_REACHABILITY_AREA_INDEX:	//(vec3_t point)
		return syscall_->AASPointReachabilityAreaIndex((vec_t *)ptr(0));
	case BOTLIB_AAS_TRACE_AREAS: //(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas)
		return syscall_->AASTraceAreas((vec_t *)ptr(0), (vec_t *)ptr(1), (int *)ptr(2), (vec3_t *)ptr(3), arg(4));
	case BOTLIB_AAS_BBOX_AREAS:	//(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas)
		return syscall_->AASBBoxAreas((vec_t *)ptr(0), (vec_t *)ptr(1), (int *)ptr(2), arg(3));
	case BOTLIB_AAS_AREA_INFO: //( int areanum, void /* struct aas_areainfo_s */ *info )
		return syscall_->AASAreaInfo(arg(0), ptr(1));
	case BOTLIB_AAS_POINT_CONTENTS:	//(vec3_t point)
		return syscall_->AASPointContents((vec_t *)ptr(0));
	case BOTLIB_AAS_NEXT_BSP_ENTITY: //(int ent)
		return syscall_->AASNextBSPEntity(arg(0));
	case BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY: //(int ent, char *key, char *value, int size)
		return syscall_->AASValueForBSPEpairKey(arg(0), (char *)ptr(1), (char *)ptr(2), arg(3));
	case BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY: //(int ent, char *key, vec3_t v)
		return syscall_->AASVectorForBSPEpairKey(arg(0), (char *)ptr(1), (vec_t  *)ptr(2));
	case BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY: //(int ent, char *key, float *value)
		return syscall_->AASFloatForBSPEpairKey(arg(0), (char *)ptr(1), (float *)ptr(2));
	case BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY: //(int ent, char *key, int *value)
		return syscall_->AASIntForBSPEpairKey(arg(0), (char *)ptr(1), (int *)ptr(2));
	case BOTLIB_AAS_AREA_REACHABILITY: //(int areanum)
		return syscall_->AASAreaReachability(arg(0));
	case BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA: //(int areanum, vec3_t origin, int goalareanum, int travelflags)
		return syscall_->AASAreaTravelTimeToGoalArea(arg(0), (vec_t *)ptr(1), arg(2), arg(3));
	case BOTLIB_AAS_ENABLE_ROUTING_AREA: //(int areanum, int enable)
		return syscall_->AASEnableRoutingArea(arg(0), arg(1));
	case BOTLIB_AAS_PREDICT_ROUTE: //(void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin, int goalareanum, int travelflags, int maxareas, int maxtime, int stopevent, int stopcontents, int stoptfl, int stopareanum)
		return syscall_->AASPredictRoute(ptr(0), arg(1), (vec_t *)ptr(2), arg(3), arg(4), arg(5), arg(6), arg(7), arg(8), arg(9), arg(10));
	case BOTLIB_AAS_ALTERNATIVE_ROUTE_GOAL:	//(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags, void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals, int type)
		return syscall_->AASAlternativeRouteGoals((vec_t *)ptr(0), arg(1), (vec_t *)ptr(2), arg(3), arg(4), ptr(5), arg(6), arg(7));
	case BOTLIB_AAS_SWIMMING: //(vec3_t origin)
		return syscall_->AASSwimming((vec_t *)ptr(0));
	case BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT: //(void /* struct aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize)
		return syscall_->AASPredictClientMovement(ptr(0), arg(1), (vec_t *)ptr(2), arg(3), arg(4), (vec_t *)ptr(5), (vec_t *)ptr(6), arg(7), arg(8), VMF(9), arg(10), arg(11), arg(12));
	case BOTLIB_EA_SAY:	//(int client, char *str)
		syscall_->EASay(arg(0), (char *)ptr(1));
		return 0;
	case BOTLIB_EA_SAY_TEAM: //(int client, char *str)
		syscall_->EASayTeam(arg(0), (char *)ptr(1));
		return 0;
	case BOTLIB_EA_COMMAND:	//(int client, char *command)
		syscall_->EACommand(arg(0), (char *)ptr(1));
		return 0;
	case BOTLIB_EA_ACTION: //(int client, int action)
		syscall_->EAAction(arg(0), arg(1));
		return 0;
	case BOTLIB_EA_GESTURE:	//(int client)
		syscall_->EAGesture(arg(0));
		return 0;
	case BOTLIB_EA_TALK: //(int client)
		syscall_->EATalk(arg(0));
		return 0;
	case BOTLIB_EA_ATTACK: //(int client)
		syscall_->EAAttack(arg(0));
		return 0;
	case BOTLIB_EA_USE: //(int client)
		syscall_->EAUse(arg(0));
		return 0;
	case BOTLIB_EA_RESPAWN: //(int client)
		syscall_->EARespawn(arg(0));
		return 0;
	case BOTLIB_EA_CROUCH: //(int client)
		syscall_->EACrouch(arg(0));
		return 0;
	case BOTLIB_EA_MOVE_UP: //(int client)
		syscall_->EAMoveUp(arg(0));
		return 0;
	case BOTLIB_EA_MOVE_DOWN: //(int client)
		syscall_->EAMoveDown(arg(0));
		return 0;
	case BOTLIB_EA_MOVE_FORWARD: //(int client)
		syscall_->EAMoveForward(arg(0));
		return 0;
	case BOTLIB_EA_MOVE_BACK: //(int client)
		syscall_->EAMoveBack(arg(0));
		return 0;
	case BOTLIB_EA_MOVE_LEFT: //(int client)
		syscall_->EAMoveLeft(arg(0));
		return 0;
	case BOTLIB_EA_MOVE_RIGHT: //(int client)
		syscall_->EAMoveRight(arg(0));
		return 0;
	case BOTLIB_EA_SELECT_WEAPON: //(int client, int weapon)
		syscall_->EASelectWeapon(arg(0), arg(1));
		return 0;
	case BOTLIB_EA_JUMP: //(int client)
		syscall_->EAJump(arg(0));
		return 0;
	case BOTLIB_EA_DELAYED_JUMP: //(int client)
		syscall_->EADelayedJump(arg(0));
		return 0;
	case BOTLIB_EA_MOVE: //(int client, vec3_t dir, float speed)
		syscall_->EAMove(arg(0), (vec_t *)ptr(1), VMF(2));
		return 0;
	case BOTLIB_EA_VIEW: //(int client, vec3_t viewangles)
		syscall_->EAView(arg(0), (vec_t *)ptr(1));
		return 0;
	case BOTLIB_EA_END_REGULAR:	//(int client, float thinktime)
		syscall_->EAEndRegular(arg(0), VMF(1));
		return 0;
	case BOTLIB_EA_GET_INPUT: //(int client, float thinktime, void /* struct bot_input_s */ *input)
		syscall_->EAGetInput(arg(0), VMF(1), ptr(2));
		return 0;
	case BOTLIB_EA_RESET_INPUT:	//(int client)
		syscall_->EAResetInput(arg(0));
		return 0;
	case BOTLIB_AI_LOAD_CHARACTER: //(char *charfile, float skill)
		return syscall_->BotLoadCharacter((char *)ptr(0), VMF(1));
	case BOTLIB_AI_FREE_CHARACTER: //(int character)
		syscall_->BotFreeCharacter(arg(0));
		return 0;
	case BOTLIB_AI_CHARACTERISTIC_FLOAT: //(int character, int index)
		return PASSFLOAT2(syscall_->CharacteristicFloat(arg(0), arg(1)));
	case BOTLIB_AI_CHARACTERISTIC_BFLOAT: //(int character, int index, float min, float max)
		return PASSFLOAT2(syscall_->CharacteristicBFloat(arg(0), arg(1), VMF(2), VMF(3)));
	case BOTLIB_AI_CHARACTERISTIC_INTEGER: //(int character, int index)
		return syscall_->CharacteristicInteger(arg(0), arg(1));
	case BOTLIB_AI_CHARACTERISTIC_BINTEGER:	//(int character, int index, int min, int max)
		return syscall_->CharacteristicBInteger(arg(0), arg(1), arg(2), arg(3));
	case BOTLIB_AI_CHARACTERISTIC_STRING: //(int character, int index, char *buf, int size)
		syscall_->CharacteristicString(arg(0), arg(1), (char *)ptr(2), arg(3));
		return 0;
	case BOTLIB_AI_ALLOC_CHAT_STATE: //(void)
		return syscall_->BotAllocChatState();
	case BOTLIB_AI_FREE_CHAT_STATE:	//(int handle)
		syscall_->BotFreeChatState(arg(0));
		return 0;
	case BOTLIB_AI_QUEUE_CONSOLE_MESSAGE: //(int chatstate, int type, char *message)
		syscall_->BotQueueConsoleMessage(arg(0), arg(1), (char *)ptr(2));
		return 0;
	case BOTLIB_AI_REMOVE_CONSOLE_MESSAGE://(int chatstate, int handle)
		syscall_->BotRemoveConsoleMessage(arg(0), arg(1));
		return 0;
	case BOTLIB_AI_NEXT_CONSOLE_MESSAGE: //(int chatstate, void /* struct bot_consolemessage_s */ *cm)
		return syscall_->BotNextConsoleMessage(arg(0), ptr(1));
	case BOTLIB_AI_NUM_CONSOLE_MESSAGE:	//(int chatstate)
		return syscall_->BotNumConsoleMessages(arg(0));
	case BOTLIB_AI_INITIAL_CHAT: //(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 )
		syscall_->BotInitialChat(arg(0), (char *)ptr(1), arg(2), (char *)ptr(3), (char *)ptr(4), (char *)ptr(5), (char *)ptr(6), (char *)ptr(7), (char *)ptr(8), (char *)ptr(9), (char *)ptr(10));
		return 0;
	case BOTLIB_AI_NUM_INITIAL_CHATS: //(int chatstate, char *type)
		return syscall_->BotNumInitialChats(arg(0), (char *)ptr(1));
	case BOTLIB_AI_REPLY_CHAT: //(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 )
		return syscall_->BotReplyChat(arg(0), (char *)ptr(1), arg(2), arg(3), (char *)ptr(4), (char *)ptr(5), (char *)ptr(6), (char *)ptr(7), (char *)ptr(8), (char *)ptr(9), (char *)ptr(10), (char *)ptr(11));
	case BOTLIB_AI_CHAT_LENGTH: //(int chatstate)
		return syscall_->BotChatLength(arg(0));
	case BOTLIB_AI_ENTER_CHAT: //(int chatstate, int client, int sendto)
		syscall_->BotEnterChat(arg(0), arg(1), arg(2));
		return 0;
	case BOTLIB_AI_GET_CHAT_MESSAGE: //(int chatstate, char *buf, int size)
		syscall_->BotGetChatMessage(arg(0), (char *)ptr(1), arg(2));
		return 0;
	case BOTLIB_AI_STRING_CONTAINS:	//(char *str1, char *str2, int casesensitive)
		return syscall_->StringContains((char *)ptr(0), (char *)ptr(1), arg(2));
	case BOTLIB_AI_FIND_MATCH: //(char *str, void /* struct bot_match_s */ *match, unsigned long int context)
		return syscall_->BotFindMatch((char *)ptr(0), (char *)ptr(1), arg(2));
	case BOTLIB_AI_MATCH_VARIABLE: //(void /* struct bot_match_s */ *match, int variable, char *buf, int size)
		syscall_->BotMatchVariable(ptr(0), arg(1), (char *)ptr(2), arg(3));
		return 0;
	case BOTLIB_AI_UNIFY_WHITE_SPACES: //(char* string)
		syscall_->UnifyWhiteSpaces((char *)ptr(0));
		return 0;
	case BOTLIB_AI_REPLACE_SYNONYMS: //(char *string, unsigned long int context)
		syscall_->BotReplaceSynonyms((char *)ptr(0), arg(1));
		return 0;
	case BOTLIB_AI_LOAD_CHAT_FILE: //(int chatstate, char* chatfile, char* chatname)
		return syscall_->BotLoadChatFile(arg(0), (char *)ptr(1), (char *)ptr(2));
	case BOTLIB_AI_SET_CHAT_GENDER: //(int chatstate, int gender)
		syscall_->BotSetChatGender(arg(0), arg(1));
		return 0;
	case BOTLIB_AI_SET_CHAT_NAME: //(int chatstate, char *name, int client)
		syscall_->BotSetChatName(arg(0), (char *)ptr(1), arg(2));
		return 0;
	case BOTLIB_AI_RESET_GOAL_STATE: //(int goalstate)
		syscall_->BotResetGoalState(arg(0));
		return 0;
	case BOTLIB_AI_RESET_AVOID_GOALS: //(int goalstate)
		syscall_->BotResetAvoidGoals(arg(0));
		return 0;
	case BOTLIB_AI_REMOVE_FROM_AVOID_GOALS:	//(int goalstate, int number)
		syscall_->BotResetAvoidGoals(arg(0));
		return 0;
	case BOTLIB_AI_PUSH_GOAL: //(int goalstate, void* goal)
		syscall_->BotPushGoal(arg(0), ptr(1));
		return 0;
	case BOTLIB_AI_POP_GOAL: //(int goalstate)
		syscall_->BotPopGoal(arg(0));
		return 0;
	case BOTLIB_AI_EMPTY_GOAL_STACK: //(int goalstate)
		syscall_->BotEmptyGoalStack(arg(0));
		return 0;
	case BOTLIB_AI_DUMP_AVOID_GOALS: //(int goalstate)
		syscall_->BotDumpAvoidGoals(arg(0));
		return 0;
	case BOTLIB_AI_DUMP_GOAL_STACK: //(int goalstate)
		syscall_->BotDumpGoalStack(arg(0));
		return 0;
	case BOTLIB_AI_GOAL_NAME: //(int number, char *name, int size)
		syscall_->BotGoalName(arg(0), (char *)ptr(1), arg(2));
		return 0;
	case BOTLIB_AI_GET_TOP_GOAL: //(int goalstate, void /* struct bot_goal_s */ *goal)
		return syscall_->BotGetTopGoal(arg(0), ptr(1));
	case BOTLIB_AI_GET_SECOND_GOAL: //(int goalstate, void /* struct bot_goal_s */ *goal)
		return syscall_->BotGetSecondGoal(arg(0), ptr(1));
	case BOTLIB_AI_CHOOSE_LTG_ITEM:	//(int goalstate, vec3_t origin, int *inventory, int travelflags)
		return syscall_->BotChooseLTGItem(arg(0), (vec_t *)ptr(1), (int *)ptr(2), arg(3));
	case BOTLIB_AI_CHOOSE_NBG_ITEM:	//(int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime)
		return syscall_->BotChooseNBGItem(arg(0), (vec_t *)ptr(1), (int *)ptr(2), arg(3), ptr(4), VMF(5));
	case BOTLIB_AI_TOUCHING_GOAL: //(vec3_t origin, void /* struct bot_goal_s */ *goal)
		return syscall_->BotTouchingGoal((vec_t *)ptr(0), ptr(1));
	case BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE: //(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal)
		return syscall_->BotItemGoalInVisButNotVisible(arg(0), (vec_t *)ptr(1), (vec_t *)ptr(2), ptr(3));
	case BOTLIB_AI_GET_LEVEL_ITEM_GOAL:	//(int index, char *classname, void /* struct bot_goal_s */ *goal)
		return syscall_->BotGetLevelItemGoal(arg(0), (char *)ptr(1), ptr(2));
	case BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL:	//(int num, void /* struct bot_goal_s */ *goal)
		return syscall_->BotGetNextCampSpotGoal(arg(0), ptr(1));
	case BOTLIB_AI_GET_MAP_LOCATION_GOAL: //(char *name, void /* struct bot_goal_s */ *goal)
		return syscall_->BotGetMapLocationGoal((char *)ptr(0), ptr(1));
	case BOTLIB_AI_AVOID_GOAL_TIME: //(int goalstate, int number)
		return PASSFLOAT2(syscall_->BotAvoidGoalTime(arg(0), arg(1)));
	case BOTLIB_AI_SET_AVOID_GOAL_TIME:	//(int goalstate, int number, float avoidtime)
		syscall_->BotSetAvoidGoalTime(arg(0), arg(1), VMF(2));
		return 0;
	case BOTLIB_AI_INIT_LEVEL_ITEMS: //(void)
		syscall_->BotInitLevelItems();
		return 0;
	case BOTLIB_AI_UPDATE_ENTITY_ITEMS:	//(void)
		syscall_->BotUpdateEntityItems();
		return 0;
	case BOTLIB_AI_LOAD_ITEM_WEIGHTS: //(int, char*)
		return syscall_->BotLoadItemWeights(arg(0), (char *)ptr(1));
	case BOTLIB_AI_FREE_ITEM_WEIGHTS: //(int goalstate)
		syscall_->BotFreeItemWeights(arg(0));
		return 0;
	case BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC:	//(int,int,int)
		syscall_->BotInterbreedGoalFuzzyLogic(arg(0), arg(1), arg(2));
		return 0;
	case BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC: //(int, char*)
		syscall_->BotSaveGoalFuzzyLogic(arg(0), (char *)ptr(1));
		return 0;
	case BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC: //(int goalstate, float range)
		syscall_->BotMutateGoalFuzzyLogic(arg(0), VMF(1));
		return 0;
	case BOTLIB_AI_ALLOC_GOAL_STATE: //(int state)
		return syscall_->BotAllocGoalState(arg(0));
	case BOTLIB_AI_FREE_GOAL_STATE: //(int handle)
		syscall_->BotFreeGoalState(arg(0));
		return 0;
	case BOTLIB_AI_RESET_MOVE_STATE: //(int movestate)
		syscall_->BotResetMoveState(arg(0));
		return 0;
	case BOTLIB_AI_ADD_AVOID_SPOT: //(int movestate, vec3_t origin, float radius, int type)
		syscall_->BotAddAvoidSpot(arg(0), (vec_t *)ptr(1), VMF(2), arg(3));
		return 0;
	case BOTLIB_AI_MOVE_TO_GOAL: //(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags)
		syscall_->BotMoveToGoal(ptr(0), arg(1), ptr(2), arg(3));
		return 0;
	case BOTLIB_AI_MOVE_IN_DIRECTION: //(int movestate, vec3_t dir, float speed, int type)
		return syscall_->BotMoveInDirection(arg(0), (vec_t *)ptr(1), VMF(2), arg(3));
	case BOTLIB_AI_RESET_AVOID_REACH: //(int movestate)
		syscall_->BotResetAvoidReach(arg(0));
		return 0;
	case BOTLIB_AI_RESET_LAST_AVOID_REACH: //(int movestate)
		syscall_->BotResetLastAvoidReach(arg(0));
		return 0;
	case BOTLIB_AI_REACHABILITY_AREA: //(vec3_t origin, int testground)
		return syscall_->BotReachabilityArea((vec_t *)ptr(0), arg(1));
	case BOTLIB_AI_MOVEMENT_VIEW_TARGET: //(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target)
		return syscall_->BotMovementViewTarget(arg(0), ptr(1), arg(2), VMF(3), (vec_t *)ptr(4));
	case BOTLIB_AI_PREDICT_VISIBLE_POSITION: //(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target)
		return syscall_->BotPredictVisiblePosition((vec_t *)ptr(0), arg(1), ptr(2), arg(3), (vec_t *)ptr(4));
	case BOTLIB_AI_ALLOC_MOVE_STATE: //(void)
		return syscall_->BotAllocMoveState();
	case BOTLIB_AI_FREE_MOVE_STATE:	//(int handle)
		syscall_->BotFreeMoveState(arg(0));
		return 0;
	case BOTLIB_AI_INIT_MOVE_STATE:	//(int handle, void* initmove)
		syscall_->BotInitMoveState(arg(0), ptr(1));
		return 0;
	case BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON: //(int weaponstate, int *inventory)
		return syscall_->BotChooseBestFightWeapon(arg(0), (int *)ptr(1));
	case BOTLIB_AI_GET_WEAPON_INFO: //(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo)
		syscall_->BotGetWeaponInfo(arg(0), arg(1), ptr(2));
		return 0;
	case BOTLIB_AI_LOAD_WEAPON_WEIGHTS:	//(int, char*)
		return syscall_->BotLoadWeaponWeights(arg(0), (char *)ptr(1));
	case BOTLIB_AI_ALLOC_WEAPON_STATE: //(void)
		return syscall_->BotAllocWeaponState();
	case BOTLIB_AI_FREE_WEAPON_STATE: //(int)
		syscall_->BotFreeWeaponState(arg(0));
		return 0;
	case BOTLIB_AI_RESET_WEAPON_STATE: //(int)
		syscall_->BotResetWeaponState(arg(0));
		return 0;
	case BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION:	//(int numranks, float *ranks, int *parent1, int *parent2, int *child)
		return syscall_->GeneticParentsAndChildSelection(arg(0), (float *)ptr(1), (int *)ptr(2), (int *)ptr(3), (int *)ptr(4));
	case BOTLIB_PC_LOAD_SOURCE: //(const char*)
		return syscall_->PCLoadSource((const char *)ptr(0));
	case BOTLIB_PC_FREE_SOURCE:	//(int)
		return syscall_->PCFreeSource(arg(0));
	case BOTLIB_PC_READ_TOKEN: //(int, void*)
		return syscall_->PCReadToken(arg(0), (pc_token_t *)ptr(1));
	case BOTLIB_PC_SOURCE_FILE_AND_LINE: //( int handle, char *filename, int *line )
		return syscall_->PCSourceFileAndLine(arg(0), (char *)ptr(1), (int *)ptr(2));
	case G_MEMSET: //(void* dest, int c, size_t count)
		syscall_->MemSet(ptr(0), arg(1), arg(2));
		return 0;
	case G_MEMCPY: //(void *dest, const void *src, size_t count)
		syscall_->MemCpy(ptr(0), ptr(1), arg(2));
		return 0;
	case G_STRNCPY: //(char *strDest, const char *strSource, size_t count )
		return (int)syscall_->StringNCopy((char *)ptr(0), (const char *)ptr(1), arg(2));
	case G_SIN:	//(float)
		return PASSFLOAT2(syscall_->Sin(VMF(0)));
		//return syscall_->syscall_(G_SIN, arg(0));
	case G_COS:	//(float)
		return PASSFLOAT2(syscall_->Cos(VMF(0)));
		//return syscall_->syscall_(G_COS, arg(0));
	case G_ATAN2: //(float, float)
		return PASSFLOAT2(syscall_->ATan2(VMF(0), VMF(1)));
		//return syscall_->syscall_(G_ATAN2, arg(0));
	case G_SQRT: //(float)
		return PASSFLOAT2(syscall_->Sqrt(VMF(0)));
		//return syscall_->syscall_(G_SQRT, arg(0));
	case G_FLOOR: //(float)
		return PASSFLOAT2(syscall_->Floor(VMF(0)));
		//return syscall_->syscall_(G_FLOOR, arg(0));
	case G_CEIL: //(float)
		return PASSFLOAT2(syscall_->Ceil(VMF(0)));
		//return syscall_->syscall_(G_CEIL, arg(0));
	default: //bad trap (ignore it, print error to console)
		syscall_->PrintError("VM::SysCalls: Unhandled syscall:\n");
		return 0;
	}

	syscall_->PrintError("VM::SysCalls: Unhandled syscall\n");
	return 0;
}
