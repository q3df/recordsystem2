#ifndef CLIENT_Q3SYSCALLHOOK_H_
#define CLIENT_Q3SYSCALLHOOK_H_

#include <list>
#include "Q3SysCallHook.h"
#include "Quake3.h"
#include <functional>

enum Q3SysCallHookExecuteType
{
	EXECUTE_TYPE_BEFORE,
	EXECUTE_TYPE_AFTER
};

class Q3SysCallHook {
friend class Q3SysCall;
public:
	Q3SysCallHook(gameImport_t eventType, Q3SysCallHookExecuteType executeType, std::function<void(Q3SysCallHook *)>);
	~Q3SysCallHook();

	int getParam(int index);
	float getParamFloat(int index);
	void *getParamPtr(int index);

	void setParam(int index, int arg);
	void setParam(int index, void *arg);
	void setParam(int index, float arg);

	int getReturnArg();
	void *getReturnVMA();
	float getReturnVMF();
	
	void setReturnArg(int retArg);
	void setReturnVMA(void *retPtr);
	void setReturnVMF(float retArg);
	
private:
	int arg_[11];
	int eventType_;
	int executeType_;
	int returnValue_;
	
	void reset();
	std::function<void(Q3SysCallHook *)> callback_;
	void executeCallback() { this->callback_(this);	};
};

#endif // CLIENT_Q3SYSCALLHOOK_H_
