#ifndef CLIENT_APIASYNCEXECUTER_H_
#define CLIENT_APIASYNCEXECUTER_H_

#include <list>
#include <queue>
#include <functional>
#include <pthread.h>

#include <google/protobuf/message.h>
#include <google/protobuf/rpc/rpc_client.h>

typedef std::function<::google::protobuf::rpc::Error ()> ExecuterFunction;
typedef std::function<void(::google::protobuf::Message *, ::google::protobuf::rpc::Error *)> ExecuterCallbackFunction;

class ApiAsyncItem {
public:
	ApiAsyncItem(ExecuterFunction item, ExecuterCallbackFunction callback, ::google::protobuf::Message *replyMsg)
		: callback_(callback),
		  function_(item),
		  replyMsg_(replyMsg) {
	};

	~ApiAsyncItem() {
		delete replyMsg_;
	};

	void ExecuteApi() {
		error = function_();
	}

	void ExecuteCallback() {
		callback_(replyMsg_, &error);
	};

private:
	ExecuterCallbackFunction callback_;
	ExecuterFunction function_;
	::google::protobuf::Message *replyMsg_;
	::google::protobuf::rpc::Error error;
};

class ApiAsyncExecuter {
public:
	ApiAsyncExecuter();
	~ApiAsyncExecuter();

	void ExecuteAsync(ExecuterFunction item, ::google::protobuf::Message *replyMsg, ExecuterCallbackFunction callback);
	void DoMainThreadWork();

private:
	void BackgroundWorker();
	static void *BackgroundWorkerStart(void *ctx);
	
	bool stoppingThread_;
	pthread_cond_t cond_;
	pthread_mutex_t mutex_;
	pthread_mutex_t mutex2_;
	pthread_t thread_;
	std::queue<ApiAsyncItem *> itemsThread;
	std::queue<ApiAsyncItem *> itemsMainThread;
};

#endif // CLIENT_APIASYNCEXECUTER_H_
