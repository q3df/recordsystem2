#include <list>
#include <queue>
#include <google/protobuf/message.h>
#include <functional>
#include <pthread.h>

class ApiAsyncExecuterItem {
public:
	ApiAsyncExecuterItem() {};
	~ApiAsyncExecuterItem() {
		delete msg;
	};

	void ExecuteCallback(::google::protobuf::Message *rplMsg) {
		callbackDone(rplMsg);
	};

private:
	::google::protobuf::Message *msg;
	std::function<void(::google::protobuf::Message *)> callbackDone;
};

class ApiAsyncExecuter {
public:
	ApiAsyncExecuter();
	~ApiAsyncExecuter();

	void ExecuteAsync(::google::protobuf::Message *msg, std::function<void(::google::protobuf::Message *)>callbackDone);
private:
	void BackgroundWorker();
	std::queue<ApiAsyncExecuterItem*> items;
};