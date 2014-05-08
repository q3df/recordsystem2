#include "ApiAsyncExecuter.h"
#include "Recordsystem.h"

void ApiAsyncItem::ExecuteCallback() {
	if(callback_)
		callback_(replyMsg_, &error);
	else if(!error.IsNil())
		RS_PrintError(va("%s\n", error.String().c_str()));
}

ApiAsyncExecuter::ApiAsyncExecuter() : stoppingThread_(false) {
	mutex_ = PTHREAD_MUTEX_INITIALIZER;
	mutex2_ = PTHREAD_MUTEX_INITIALIZER;
	cond_ = PTHREAD_COND_INITIALIZER;

	pthread_create(&thread_, NULL, &ApiAsyncExecuter::BackgroundWorkerStart, this);
}

ApiAsyncExecuter::~ApiAsyncExecuter() {
	stoppingThread_ = true;
	pthread_cond_signal(&cond_);
	pthread_join(thread_, NULL);

	DoMainThreadWork();

	pthread_cond_destroy(&cond_);
	pthread_mutex_destroy(&mutex_);
	pthread_mutex_destroy(&mutex2_);

	if(!itemsThread_.empty())
		RS_PrintWarning("ApiAsyncExecuter.~ApiAsyncExecuter.itemsThread is not empty!\n");

	if(!itemsMainThread_.empty())
		RS_PrintWarning("ApiAsyncExecuter.~ApiAsyncExecuter.itemsMainThread is not empty!\n");
}

void ApiAsyncExecuter::ExecuteAsync(ExecuterFunction item, ::google::protobuf::Message *replyMsg, ::google::protobuf::Message *sentMsg, ExecuterCallbackFunction callback) {
		pthread_mutex_lock(&mutex_);
		itemsThread_.push(new ApiAsyncItem(item, callback, replyMsg, sentMsg));
		pthread_mutex_unlock(&mutex_);

		pthread_cond_signal(&cond_);
}

void ApiAsyncExecuter::DoMainThreadWork() {
	pthread_mutex_lock(&mutex2_);
	while(!itemsMainThread_.empty()) {
		ApiAsyncItem *item = NULL;
		item = itemsMainThread_.front();
		itemsMainThread_.pop();

		item->ExecuteCallback();
		delete item;
	}
	pthread_mutex_unlock(&mutex2_);
}

void ApiAsyncExecuter::BackgroundWorker() {
	while(!stoppingThread_) {
		pthread_mutex_lock(&mutex_);

		while(itemsThread_.empty() && !stoppingThread_) {
			pthread_cond_wait(&cond_, &mutex_);
		}

		while(!itemsThread_.empty()) {
			ApiAsyncItem *item = itemsThread_.front();
			itemsThread_.pop();
			pthread_mutex_unlock(&mutex_);

			if(item) {
				item->ExecuteApi();
				pthread_mutex_lock(&mutex2_);
				itemsMainThread_.push(item);
				pthread_mutex_unlock(&mutex2_);
			}
		}
	}
}

void *ApiAsyncExecuter::BackgroundWorkerStart(void *args) {
	((ApiAsyncExecuter *)args)->BackgroundWorker();

	return 0;
}
