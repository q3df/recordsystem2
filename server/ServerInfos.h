// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_SERVERINFOS_H_
#define SERVER_SERVERINFOS_H_

#include <string>
#include <map>
#include <list>
#include <vector>
#include <sstream>

#include <google/protobuf/rpc/rpc_conn.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

#include "Console.h"

using namespace std;
using namespace google::protobuf::rpc;

class ServerInfos {
public:
	ServerInfos(int serverId, const string &serverInfo);
	~ServerInfos();
	string GetServerInfo(string key);
	const string *GetServerKey();
	int GetServerId();

	static void Init();
	static void Dispose();

	static void InsertInfo(Conn *con, int serverId, const string &serverInfo);
	static void DeleteInfo(Conn *con);
	static void PrintList(Console *con);

private:
	vector<string> &split(const string &s, char delim, vector<string> &elems);	
	void split(const string &s, char delim);

	map<string, string> info_;
	string *serverKey_;
	int serverId_;
	Conn *con_;
};

typedef map<Conn*, ServerInfos*> ServerInfosMap;
typedef ServerInfosMap::iterator ServerInfosMapIterator;

#endif // SERVER_SERVERINFOS_H