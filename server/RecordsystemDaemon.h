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
#include <server/Commands.h>

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

typedef map<string, string> SettingsMap;
typedef map<string, string>::iterator SettingsMapIterator;

class GlobalObject {
private:
	SettingsMap settings_;
	ClientList *clientList_;
	MysqlPool *mysqlPool_;
	Console *con_;
	Q3dfEnv *env_;
	vector<CommandBase*> *cmdList_;

public:
	GlobalObject();
	~GlobalObject();

	void Initialize(string mysql_host, string mysql_username, string mysql_password, string mysql_dbname, int poolSize);

	ClientList *Clients();

	SettingsMap Settings();
	void SetSetting(string key, string value);
	const string &GetSetting(string key);
	bool HasSettingKey(string key);
	void PrintSettingsList();
	bool  HandleCommand(string const& cmd, const vector<string>* args, Conn *contextCon, string *output);
	MysqlPool *SqlPool();
	Console *Con();
	Q3dfEnv *Env();
};

extern GlobalObject *RS;

#endif // SERVER_RECORDSYSTEMDAEMON_H_
