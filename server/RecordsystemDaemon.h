// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_RECORDSYSTEMDAEMON_H_
#define SERVER_RECORDSYSTEMDAEMON_H_

#include <cstdarg>
#include <pthread.h>
#include <list>
#include <map>
#include <q3df_api.pb.h>

#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

#include "ClientList.h"
#include "MysqlPool.h"
#include "Console.h"
#include "Q3dfEnv.h"

#include <mysql_public_iface.h>

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

#ifndef WIN32
#	include <unistd.h>
#	define Sleep(x) usleep(x*1000)
#endif

extern "C" {
	const char *va( const char *format, ... );
}

typedef std::map<string, string> SettingsMap;
typedef std::map<string, string>::iterator SettingsMapIterator;

class GlobalObject {
private:
	SettingsMap settings_;
	ClientList *clientList_;
	MysqlPool *mysqlPool_;
	Console *con_;
	Q3dfEnv *env_;

public:
	GlobalObject(string mysql_host, string mysql_username, string mysql_password, string mysql_dbname, int poolSize);
	~GlobalObject();

	ClientList *Clients() { return this->clientList_; }
	SettingsMap Settings() { return this->settings_; }

	void SetSetting(string key, string value) {
		settings_[key].clear();
		settings_[key].append(value);
	}

	const string &GetSetting(string key) {
		return settings_[key];
	}

	bool HasSettingKey(string key) {
		return settings_.find(key) != settings_.end();
	}

	void PrintSettingsList() {
		SettingsMapIterator it;
		this->Con()->Print("  Settings-List\n");
		this->Con()->Print(" ^3---------------------------------------^7\n");
		for (it=settings_.begin(); it!=settings_.end(); ++it)
			this->Con()->Print("    %s = '%s'\n", it->first.c_str(), it->second.c_str());
	}

	MysqlPool *SqlPool() { return this->mysqlPool_; }
	Console *Con() { return this->con_; }
	Q3dfEnv *Env() { return this->env_; }
};

extern GlobalObject *RS;

#endif // SERVER_RECORDSYSTEMDAEMON_H_
