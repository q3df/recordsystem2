// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_RECORDSYSTEMDAEMON_H_
#define SERVER_RECORDSYSTEMDAEMON_H_

#include <cstdarg>
#include <pthread.h>
#include <list>
#include <pthread.h>

#include <q3df_api.pb.h>

#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

#include "Console.h"
#include "Q3dfEnv.h"

#include <mysql_public_iface.h>
#include "MysqlPool.h"

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

#ifdef WIN32
#	include "ConsoleWin32.h"
#else
#	include <unistd.h>
#	include "ConsoleTty.h"
#	define Sleep(x) usleep(x*1000)
#endif

const char *va( const char *format, ... );

typedef std::map<string, string> SettingsMap;
typedef std::map<string, string>::iterator SettingsMapIterator;
extern SettingsMap gSettings;

extern std::list<Conn *> gClientList;
extern MysqlPool *gMysqlPool;

#endif // SERVER_RECORDSYSTEMDAEMON_H_
