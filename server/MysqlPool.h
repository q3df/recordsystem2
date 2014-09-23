// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_MYSQLPOOL_H_
#define SERVER_MYSQLPOOL_H_

#include <pthread.h>
#include <semaphore.h>
#include <functional>
#include <vector>
#include <exception>
#include <mysql_public_iface.h>

class MysqlPoolTwiceReturnException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "you returned a object twice ;(";
  }
};


class MysqlPool {
public:
	MysqlPool(int size, std::function<sql::Connection *()> callbackInitObject);
	~MysqlPool();

	sql::Connection * Get();

	void Return(sql::Connection * obj);

private:
	bool IsObjInList(sql::Connection *obj);

	std::vector<sql::Connection *> objList_;
	int currentValue_;
	sem_t mutex_;
	int size_;
};

#endif // SERVER_MYSQLPOOL_H_