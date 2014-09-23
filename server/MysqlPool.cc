// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MysqlPool.h"

MysqlPool::MysqlPool(int size, std::function<sql::Connection *()> callbackInitObject) {
	int i = 0;
	this->size_ = size;
	sem_init(&this->mutex_, 0, this->size_);

	for(i = 0; i<this->size_; i++) {
		this->objList_.push_back(callbackInitObject());
	}
}

MysqlPool::~MysqlPool() {
	sql::Connection *con;

	sem_destroy(&this->mutex_);
	
	while(this->objList_.size() != 0) {
		con = this->objList_.back();
		this->objList_.pop_back();
		con->close();
		delete con;
	}
}


sql::Connection *MysqlPool::Get() {
	sem_wait(&this->mutex_);

	sql::Connection *ret = this->objList_.back();
	this->objList_.pop_back();

	return ret;
}

void MysqlPool::Return(sql::Connection *obj) {
	if(!this->IsObjInList(obj)) {
		this->objList_.push_back(obj);
		sem_post(&this->mutex_);
	} else {
		// we should print a warning that a user is being returning a obj twice ;(
		//MysqlPoolTwiceReturnException t();
		//throw t;
	}
}

bool MysqlPool::IsObjInList(sql::Connection *obj) {
	for(unsigned i=0; i<this->objList_.size(); i++) {
		if(obj == this->objList_.at(i))
			return true;
	}

	return false;
}