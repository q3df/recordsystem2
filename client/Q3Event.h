#ifndef CLIENT_Q3EVENT_H_
#define CLIENT_Q3EVENT_H_

#include <functional>
#include <list>

enum Q3EventExecuteType
{
	EXECUTE_TYPE_BEFORE,
	EXECUTE_TYPE_AFTER
};

enum Q3EventReturnType
{
	HOOK_RETURN_TYPE_VMF,
	HOOK_RETURN_TYPE_VMA,
	HOOK_RETURN_TYPE_ARG
};

class Q3EventArgs {
public:
	Q3EventArgs() { returnValue_ = 0; isHandled_ = 0; memset(arg_, 0, sizeof(arg_)); }
	~Q3EventArgs() {}

	int GetParam(int index) { return arg_[index]; }
	void SetParam(int index, int arg) { arg_[index] = arg; }

	void *GetParamVMA(int index) { return (void *)arg_[index]; }
	void SetParam(int index, void *arg) { arg_[index] = (int)arg; }

	float GetParamVMF(int index) { return (*(float*)&arg_[index]); }
	void SetParam(int index, float arg) { arg_[index] = *(int *)&arg; }


	int GetReturn() { return returnValue_; }
	void SetReturn(int retArg) { returnValue_ = retArg; }

	void *GetReturnVMA() { return (void *)returnValue_; }
	void SetReturn(void *retPtr) { returnValue_ = (int)retPtr; }

	float GetReturnVMF() { return (*(float*)&returnValue_); }
	void SetReturn(float retArg) { returnValue_ = *(int *)&retArg; }

	bool GetHandled() { return isHandled_; }
	void SetHandled(bool handled) { isHandled_ = handled; }

private:
	int arg_[12];
	int returnValue_;
	bool isHandled_;
};

class Q3EventHandler {
public:
	Q3EventHandler(int eventType, Q3EventExecuteType executeType, std::function<void(Q3EventArgs *)> callback) {
		eventType_ = eventType;
		executeType_ = executeType;
		callback_ = callback;
	}

	~Q3EventHandler() {}

	int GetEventType() { return eventType_; }
	int GetExecuteType() { return executeType_; }

	void SetHandled(bool handled) { isHandled_ = handled; }
	bool GetHandled() { return isHandled_; }

	void Reset() { isHandled_ = false; }
	void Call(Q3EventArgs *e) { this->callback_(e); };

private:
	int eventType_;
	int executeType_;
	bool isHandled_;

	std::function<void(Q3EventArgs *)> callback_;
};

typedef std::list<Q3EventHandler*>::iterator EventHandlersIterator;
typedef std::list<Q3EventHandler*> EventHandlers;


#define EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	{																															\
		EventHandlersIterator it;																								\
		Q3EventArgs *e = new Q3EventArgs();																						\
		e->SetParam(0, (arg1));																									\
		e->SetParam(1, (arg2));																									\
		e->SetParam(2, (arg3));																									\
		e->SetParam(3, (arg4));																									\
		e->SetParam(4, (arg5));																									\
		e->SetParam(5, (arg6));																									\
		e->SetParam(6, (arg7));																									\
		e->SetParam(7, (arg8));																									\
		e->SetParam(8, (arg9));																									\
		e->SetParam(9, (arg10));																								\
		e->SetParam(10, (arg11));																								\
		e->SetParam(11, (arg12));																								\
		for (it = eventList_.begin(); it != eventList_.end(); it++) {															\
			if( (*it)->GetEventType() == (eventType) && (*it)->GetExecuteType() == (executeType)) {								\
				(*it)->Reset();																									\
				(*it)->Call(e);																									\
				eventItem = (*it);																								\
				eventItem->SetHandled(e->GetHandled());																			\
			}																													\
		}																														\
		delete e;																												\
	}


#define EXECUTE_EVENT_RETURN_ARG(eventType, executeType, returnValue, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	{																															\
		EventHandlersIterator it;																								\
		Q3EventArgs *e = new Q3EventArgs();																						\
		e->SetParam(0, (arg1));																									\
		e->SetParam(1, (arg2));																									\
		e->SetParam(2, (arg3));																									\
		e->SetParam(3, (arg4));																									\
		e->SetParam(4, (arg5));																									\
		e->SetParam(5, (arg6));																									\
		e->SetParam(6, (arg7));																									\
		e->SetParam(7, (arg8));																									\
		e->SetParam(8, (arg9));																									\
		e->SetParam(9, (arg10));																								\
		e->SetParam(10, (arg11));																								\
		e->SetParam(11, (arg12));																								\
		e->SetReturn((returnValue));																							\
		for (it = eventList_.begin(); it != eventList_.end(); it++) {															\
			if( (*it)->GetEventType() == (eventType) && (*it)->GetExecuteType() == (executeType)) {								\
				(*it)->Reset();																									\
				(*it)->Call(e);																									\
				eventItem = (*it);																								\
				if(e->GetReturn() != 0) {																					\
					(returnValue) = e->GetReturn();																				\
				}																												\
			}																													\
		}																														\
	}

#define EXECUTE_EVENT_RETURN_VMA(eventType, executeType, returnValue, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	{																															\
		EventHandlersIterator it;																								\
		Q3EventArgs *e = new Q3EventArgs();																						\
		e->SetParam(0, (arg1));																									\
		e->SetParam(1, (arg2));																									\
		e->SetParam(2, (arg3));																									\
		e->SetParam(3, (arg4));																									\
		e->SetParam(4, (arg5));																									\
		e->SetParam(5, (arg6));																									\
		e->SetParam(6, (arg7));																									\
		e->SetParam(7, (arg8));																									\
		e->SetParam(8, (arg9));																									\
		e->SetParam(9, (arg10));																								\
		e->SetParam(10, (arg11));																								\
		e->SetParam(11, (arg12));																								\
		e->SetReturn((void *)(returnValue));																					\
		for (it = eventList_.begin(); it != eventList_.end(); it++) {															\
			if( (*it)->GetEventType() == (eventType) && (*it)->GetExecuteType() == (executeType)) {								\
				(*it)->Reset();																									\
				(*it)->Call(e);																									\
				eventItem = (*it);																								\
				if(e->GetReturnArg() != 0) {																					\
					(returnValue) = e->GetReturnVMA();																			\
				}																												\
			}																													\
		}																														\
	}


#define EXECUTE_EVENT_RETURN_VMF(eventType, executeType, returnValue, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	{																															\
		EventHandlersIterator it;																								\
		Q3EventArgs *e = new Q3EventArgs();																						\
		e->SetParam(0, (arg1));																									\
		e->SetParam(1, (arg2));																									\
		e->SetParam(2, (arg3));																									\
		e->SetParam(3, (arg4));																									\
		e->SetParam(4, (arg5));																									\
		e->SetParam(5, (arg6));																									\
		e->SetParam(6, (arg7));																									\
		e->SetParam(7, (arg8));																									\
		e->SetParam(8, (arg9));																									\
		e->SetParam(9, (arg10));																								\
		e->SetParam(10, (arg11));																								\
		e->SetParam(11, (arg12));																								\
		e->SetReturn((float)(returnValue));																						\
		for (it = eventList_.begin(); it != eventList_.end(); it++) {															\
			if( (*it)->GetEventType() == (eventType) && (*it)->GetExecuteType() == (executeType)) {								\
				(*it)->Reset();																									\
				(*it)->Call(e);																									\
				eventItem = (*it);																								\
				if(e->GetReturnArg() != 0) {																					\
					(returnValue) = e->GetReturnVMF();																			\
				}																												\
			}																													\
		}																														\
	}

#define EXECUTE_EVENT_VOID_ARG1(eventType, executeType, arg1) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_EVENT_VOID_ARG2(eventType, executeType, arg1, arg2) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_EVENT_VOID_ARG3(eventType, executeType, arg1, arg2, arg3) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, arg3, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_EVENT_VOID_ARG4(eventType, executeType, arg1, arg2, arg3, arg4) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, arg3, arg4, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_EVENT_VOID_ARG5(eventType, executeType, arg1, arg2, arg3, arg4, arg5) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, arg3, arg4, arg5, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_EVENT_VOID_ARG6(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_EVENT_VOID_ARG7(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, 0, 0, 0, 0, 0, 0)

#define EXECUTE_EVENT_VOID_ARG8(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, 0, 0, 0, 0)

#define EXECUTE_EVENT_VOID_ARG9(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, 0, 0, 0)

#define EXECUTE_EVENT_VOID_ARG10(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, 0, 0)

#define EXECUTE_EVENT_VOID_ARG11(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, 0)

#define EXECUTE_EVENT_VOID_ARG12(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
	EXECUTE_EVENT_VOID(eventType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)


#endif // CLIENT_Q3EVENT_H_
