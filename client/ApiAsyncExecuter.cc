#include "ApiAsyncExecuter.h"

ApiAsyncExecuter::ApiAsyncExecuter() : stoppingThread_(false) {
	mutex_ = PTHREAD_MUTEX_INITIALIZER;
	mutex2_ = PTHREAD_MUTEX_INITIALIZER;
	cond_ = PTHREAD_COND_INITIALIZER;

	int ret = pthread_create(&thread_, NULL, &ApiAsyncExecuter::BackgroundWorkerStart, this);
}

ApiAsyncExecuter::~ApiAsyncExecuter() {
	stoppingThread_ = true;
	pthread_cond_signal(&cond_);
	pthread_join(thread_, NULL);
	pthread_cond_destroy(&cond_);
	pthread_mutex_destroy(&mutex_);
	pthread_mutex_destroy(&mutex2_);
}

void ApiAsyncExecuter::ExecuteAsync(ExecuterFunction item, ::google::protobuf::Message *replyMsg, ExecuterCallbackFunction callback) {
		pthread_mutex_lock(&mutex_);
		itemsThread.push(new ApiAsyncItem(item, callback, replyMsg));
		pthread_mutex_unlock(&mutex_);

		pthread_cond_signal(&cond_);
}

void ApiAsyncExecuter::DoMainThreadWork() {
	pthread_mutex_lock(&mutex2_);
	while(!itemsMainThread.empty()) {
		ApiAsyncItem *item = NULL;
		item = itemsMainThread.front();
		itemsMainThread.pop();

		item->ExecuteCallback();
		delete item;
	}
	pthread_mutex_unlock(&mutex2_);
}

void ApiAsyncExecuter::BackgroundWorker() {
	while(!stoppingThread_) {
		pthread_mutex_lock(&mutex_);

		printf("THREAD: working start waiting...\r\n");
		pthread_cond_wait(&cond_, &mutex_);
		if(stoppingThread_)
			break;

		ApiAsyncItem *item;
		if(!itemsThread.empty()) {
			item = itemsThread.front();
			itemsThread.pop();
		}
		if(item) {
			item->ExecuteApi();

			pthread_mutex_lock(&mutex2_);
			itemsMainThread.push(item);
			pthread_mutex_unlock(&mutex2_);
		}

		pthread_mutex_unlock(&mutex_);
	}
}

void *ApiAsyncExecuter::BackgroundWorkerStart(void *args) {
	((ApiAsyncExecuter *)args)->BackgroundWorker();

	return 0;
}