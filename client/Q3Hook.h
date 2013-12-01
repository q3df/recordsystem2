#ifndef CLIENT_Q3Hook_H_
#define CLIENT_Q3Hook_H_

#include <functional>
#include <map>

enum Q3HookExecuteType
{
	EXECUTE_TYPE_BEFORE,
	EXECUTE_TYPE_AFTER
};

enum Q3HookReturnType
{
	HOOK_RETURN_TYPE_VMF,
	HOOK_RETURN_TYPE_VMA,
	HOOK_RETURN_TYPE_ARG
};

class Q3Hook {
friend class Q3SysCall;
friend class Recordsystem;
public:
	Q3Hook(int eventType, Q3HookExecuteType executeType, std::function<void(Q3Hook *)>);
	~Q3Hook();

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
	void setHandled(bool handled);
	bool isHandled();

private:
	int arg_[12];
	int eventType_;
	int executeType_;
	int returnValue_;
	bool isHandled_;

	void reset();
	std::function<void(Q3Hook *)> callback_;
	void executeCallback() { this->callback_(this);	};
};
typedef std::map<Q3Hook*, Q3Hook*>::iterator HookHandlersIterator;
typedef std::map<Q3Hook*, Q3Hook*> HookHandlers;

#endif // CLIENT_Q3Hook_H_
