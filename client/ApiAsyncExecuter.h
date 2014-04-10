#ifndef CLIENT_APIASYNCEXECUTER_H_
#define CLIENT_APIASYNCEXECUTER_H_

#include <list>
#include <queue>
#include <functional>
#include <pthread.h>
#include <google/protobuf/message.h>
#include <google/protobuf/rpc/rpc_client.h>

typedef std::function<google::protobuf::rpc::Error ()> ExecuterFunction;
typedef std::function<void(google::protobuf::Message *, google::protobuf::rpc::Error *)> ExecuterCallbackFunction;

#define EXECUTE_API_ASYNC(func, sentMsg, replyMsg, callback) \
	auto bind = std::bind((func), gRecordsystem->GetQ3dfApi(),(sentMsg),(replyMsg)); \
	gRecordsystem->GetAsyncExecuter()->ExecuteAsync((bind), (replyMsg), (sentMsg), (callback));

class ApiAsyncItem {
public:
	ApiAsyncItem(ExecuterFunction item, ExecuterCallbackFunction callback, ::google::protobuf::Message *replyMsg, ::google::protobuf::Message *sentMsg)
		: callback_(callback),
		  function_(item),
		  replyMsg_(replyMsg),
		  sentMsg_(sentMsg) {
	};

	~ApiAsyncItem() {
		delete replyMsg_;
		delete sentMsg_;
	};

	void ExecuteApi() {
		error = function_();
	};

	void ExecuteCallback();

private:
	ExecuterCallbackFunction callback_;
	ExecuterFunction function_;
	::google::protobuf::Message *replyMsg_;
	::google::protobuf::Message *sentMsg_;
	::google::protobuf::rpc::Error error;
};

class ApiAsyncExecuter {
public:
	ApiAsyncExecuter();
	~ApiAsyncExecuter();

	void ExecuteAsync(ExecuterFunction item, ::google::protobuf::Message *replyMsg, ::google::protobuf::Message *sentMsg, ExecuterCallbackFunction callback);
	void DoMainThreadWork();

private:
	void BackgroundWorker();
	static void *BackgroundWorkerStart(void *ctx);
	
	bool stoppingThread_;
	pthread_cond_t cond_;
	pthread_mutex_t mutex_;
	pthread_mutex_t mutex2_;
	pthread_t thread_;
	std::queue<ApiAsyncItem *> itemsThread_;
	std::queue<ApiAsyncItem *> itemsMainThread_;
};

#endif // CLIENT_APIASYNCEXECUTER_H_
