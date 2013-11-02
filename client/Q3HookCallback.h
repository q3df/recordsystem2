#define EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	{																														\
		HookHandlers::iterator it;																							\
		for (it = hookHandlers_.begin(); it != hookHandlers_.end(); it++) {													\
			if( (*it).first->eventType_ == (hookType) && (*it).first->executeType_ == (executeType)) {						\
				(*it).first->reset();																						\
				(*it).first->setParam(0, (arg1));																			\
				(*it).first->setParam(1, (arg2));																			\
				(*it).first->setParam(2, (arg3));																			\
				(*it).first->setParam(3, (arg4));																			\
				(*it).first->setParam(4, (arg5));																			\
				(*it).first->setParam(5, (arg6));																			\
				(*it).first->setParam(6, (arg7));																			\
				(*it).first->setParam(7, (arg8));																			\
				(*it).first->setParam(8, (arg9));																			\
				(*it).first->setParam(9, (arg10));																			\
				(*it).first->setParam(10, (arg11));																			\
				(*it).first->setParam(11, (arg12));																			\
				(*it).first->executeCallback();																				\
				hook = (*it).first;																							\
			}																												\
		}																													\
	}

#define EXECUTE_CALLBACK_RETURN_ARG(hookType, executeType, returnValue, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	{																														\
		HookHandlers::iterator it;																							\
		for (it = hookHandlers_.begin(); it != hookHandlers_.end(); it++) {													\
			if( (*it).first->eventType_ == (hookType) && (*it).first->executeType_ == (executeType)) {						\
				(*it).first->reset();																						\
				(*it).first->setParam(0, (arg1));																			\
				(*it).first->setParam(1, (arg2));																			\
				(*it).first->setParam(2, (arg3));																			\
				(*it).first->setParam(3, (arg4));																			\
				(*it).first->setParam(4, (arg5));																			\
				(*it).first->setParam(5, (arg6));																			\
				(*it).first->setParam(6, (arg7));																			\
				(*it).first->setParam(7, (arg8));																			\
				(*it).first->setParam(8, (arg9));																			\
				(*it).first->setParam(9, (arg10));																			\
				(*it).first->setParam(10, (arg11));																			\
				(*it).first->setParam(11, (arg12));																			\
				(*it).first->setReturnArg((returnValue));																	\
				(*it).first->executeCallback();																				\
				hook = (*it).first;																							\
				if((*it).first->getReturnArg() != 0) {																		\
					(returnValue) = (*it).first->getReturnArg();															\
				}																											\
			}																												\
		}																													\
	}

#define EXECUTE_CALLBACK_RETURN_VMA(hookType, executeType, returnValue, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	{																														\
		HookHandlers::iterator it;																							\
		for (it = hookHandlers_.begin(); it != hookHandlers_.end(); it++) {													\
			if( (*it).first->eventType_ == (hookType) && (*it).first->executeType_ == (executeType)) {						\
				(*it).first->reset();																						\
				(*it).first->setParam(0, (arg1));																			\
				(*it).first->setParam(1, (arg2));																			\
				(*it).first->setParam(2, (arg3));																			\
				(*it).first->setParam(3, (arg4));																			\
				(*it).first->setParam(4, (arg5));																			\
				(*it).first->setParam(5, (arg6));																			\
				(*it).first->setParam(6, (arg7));																			\
				(*it).first->setParam(7, (arg8));																			\
				(*it).first->setParam(8, (arg9));																			\
				(*it).first->setParam(9, (arg10));																			\
				(*it).first->setParam(10, (arg11));																			\
				(*it).first->setParam(11, (arg12));																			\
				(*it).first->setReturnVMA((void *)(returnValue));															\
				(*it).first->executeCallback();																				\
				hook = (*it).first;																							\
				if((*it).first->getReturnArg() != 0) {																		\
					(returnValue) = (*it).first->getReturnVMA();															\
				}																											\
			}																												\
		}																													\
	}

#define EXECUTE_CALLBACK_RETURN_VMF(hookType, executeType, returnValue, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	{																														\
		HookHandlers::iterator it;																							\
		for (it = hookHandlers_.begin(); it != hookHandlers_.end(); it++) {													\
			if( (*it).first->eventType_ == (hookType) && (*it).first->executeType_ == (executeType)) {						\
				(*it).first->reset();																						\
				(*it).first->setParam(0, (arg1));																			\
				(*it).first->setParam(1, (arg2));																			\
				(*it).first->setParam(2, (arg3));																			\
				(*it).first->setParam(3, (arg4));																			\
				(*it).first->setParam(4, (arg5));																			\
				(*it).first->setParam(5, (arg6));																			\
				(*it).first->setParam(6, (arg7));																			\
				(*it).first->setParam(7, (arg8));																			\
				(*it).first->setParam(8, (arg9));																			\
				(*it).first->setParam(9, (arg10));																			\
				(*it).first->setParam(10, (arg11));																			\
				(*it).first->setParam(11, (arg12));																			\
				(*it).first->setReturnVMF((returnValue));																	\
				(*it).first->executeCallback();																				\
				hook = (*it).first;																							\
				if((*it).first->getReturnArg() != 0) {																		\
					(returnValue) = (*it).first->getReturnVMF();															\
				}																											\
			}																												\
		}																													\
	}

#define EXECUTE_CALLBACK_VOID_ARG1(hookType, executeType, arg1) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG2(hookType, executeType, arg1, arg2) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG3(hookType, executeType, arg1, arg2, arg3) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG4(hookType, executeType, arg1, arg2, arg3, arg4) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG5(hookType, executeType, arg1, arg2, arg3, arg4, arg5) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG6(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG7(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, 0, 0, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG8(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, 0, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG9(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, 0, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG10(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, 0, 0)

#define EXECUTE_CALLBACK_VOID_ARG11(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, 0)

#define EXECUTE_CALLBACK_VOID_ARG12(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
	EXECUTE_CALLBACK_VOID(hookType, executeType, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)
