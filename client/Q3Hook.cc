#include "Q3Hook.h"

Q3Hook::Q3Hook(int eventType, Q3HookExecuteType executeType, std::function<void(Q3Hook *)> callback)
  : eventType_(eventType),
    executeType_(executeType),
	isHandled_(false),
    callback_(callback) {
}

Q3Hook::~Q3Hook() {
}

void Q3Hook::setHandled(bool handled) {
	isHandled_ = handled;
}

bool Q3Hook::isHandled() {
	return isHandled_;
}

void Q3Hook::reset() {
	arg_[0] = 0;
	arg_[1] = 0;
	arg_[2] = 0;
	arg_[3] = 0;
	arg_[4] = 0;
	arg_[5] = 0;
	arg_[6] = 0;
	arg_[7] = 0;
	arg_[8] = 0;
	arg_[9] = 0;
	arg_[10] = 0;
	arg_[11] = 0;
	returnValue_ = 0;
	isHandled_ = false;
}


int Q3Hook::getParam(int index) {
	return arg_[index];
}

float Q3Hook::getParamFloat(int index) {
	return (*(float*)&arg_[index]);
}

void *Q3Hook::getParamPtr(int index) {
	return (void *)arg_[index];
}


void Q3Hook::setParam(int index, int arg) {
	arg_[index] = (int)arg;
}

void Q3Hook::setParam(int index, void *arg) {
	arg_[index] = (int)arg;
}

void Q3Hook::setParam(int index, float arg) {
	arg_[index] = *(int *)&arg;
}


int Q3Hook::getReturnArg() {
	return returnValue_;
}

void *Q3Hook::getReturnVMA() {
	return (void *)returnValue_;
}

float Q3Hook::getReturnVMF() {
	return (*(float*)&returnValue_);
}
	

void Q3Hook::setReturnArg(int retArg) {
	returnValue_ = retArg;
}

void Q3Hook::setReturnVMA(void *retPtr) {
	returnValue_ = (int)retPtr;
}

void Q3Hook::setReturnVMF(float retArg) {
	returnValue_ = *(int *)&retArg;
}
