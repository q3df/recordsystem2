// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ServerInfos.h"
#include <pthread.h>

ServerInfosMap gServerInfos;
pthread_mutex_t gServerInfosMtx = PTHREAD_MUTEX_INITIALIZER;


ServerInfos::ServerInfos(int serverId, const string &serverInfo) {
	split(serverInfo, '\\');
	serverId_ = serverId;
	serverKey_ = NULL;
}


ServerInfos::~ServerInfos() {
	info_.clear();
	if(serverKey_)
		delete serverKey_;
	con_ = NULL;
	serverId_ = 0;
}


void ServerInfos::Init() {}


void ServerInfos::Dispose() {
	for(ServerInfosMapIterator it = gServerInfos.begin(); it != gServerInfos.end(); it++) {
		it->first->Close();
		delete it->second;
	}
}


void ServerInfos::InsertInfo(Conn *con, int serverId, const string &serverInfo) {
	pthread_mutex_lock( &gServerInfosMtx );

	if(gServerInfos.find(con) == gServerInfos.end())
		gServerInfos[con] = new ServerInfos(serverId, serverInfo);

	pthread_mutex_unlock( &gServerInfosMtx );
}


void ServerInfos::DeleteInfo(Conn *con) {
	pthread_mutex_lock( &gServerInfosMtx );

	if(gServerInfos.find(con) != gServerInfos.end())
		gServerInfos.erase(con);

	pthread_mutex_unlock( &gServerInfosMtx );
}


void ServerInfos::PrintList(Console *con) {
	pthread_mutex_lock( &gServerInfosMtx );

	con->PrintInfo("ServerInfos\n");
	con->PrintInfo("-----------------------------------------------\n");
	for(ServerInfosMapIterator it = gServerInfos.begin(); it != gServerInfos.end(); it++) {
		con->PrintInfo("  %i). %s -> %s\n", it->second->GetServerId(), it->first->RemoteIpAdress(), it->second->GetServerInfo(string("sv_hostname")).c_str());
	}

	pthread_mutex_unlock( &gServerInfosMtx );
}


string ServerInfos::GetServerInfo(string key) {
	if(info_.find(key) != info_.end())
		return info_[key];

	return string("");
}


const string *ServerInfos::GetServerKey() {
	return new string("");
}


int ServerInfos::GetServerId() {
	return serverId_;
}


vector<string> &ServerInfos::split(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}

	return elems;
}
	

void ServerInfos::split(const string &s, char delim) {
	vector<string> elems;
	string item[2];

	split(s, delim, elems);

	if(elems.size() % 2 == 0) {
		for(vector<string>::iterator it = elems.begin(); it != elems.end(); it++) {
			int index = (it - elems.begin());

			item[index % 2] = *it;

			if((index % 2) > 0) {
				info_[item[0]] = item[1];
			}
		}
	}
}