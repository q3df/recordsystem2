// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_ClientList_H_
#define SERVER_ClientList_H_

#include <string>
#include <map>
#include <list>
#include <vector>
#include <sstream>

#include <google/protobuf/rpc/rpc_conn.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

using namespace std;
using namespace google::protobuf::rpc;

class ClientInfo {
public:
	ClientInfo(Conn *con, int serverId, const string &serverInfo);
	~ClientInfo();

	string GetServerInfo(string key);
	int GetServerId();
	bool IsRegistred() { return isRegistred_; }

private:
	vector<string> &split(const string &s, char delim, vector<string> &elems);	
	void split(const string &s, char delim);

	map<string, string> info_;
	int serverId_;
	bool isRegistred_;
	Conn *con_;
};

class ClientList {
public:
	ClientList();
	~ClientList();

	void Insert(Conn *con, int serverId, const string &serverInfo);
	void Delete(Conn *con);
	ClientInfo *GetClient(Conn *con);
	void ClientList::PrintList();

private:
	map<Conn*, ClientInfo*> clList_;
	pthread_mutex_t clientMapMtx_;
};

#endif // SERVER_ClientList_H_