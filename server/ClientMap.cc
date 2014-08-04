// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientMap.h"
#include <pthread.h>

ClientMapMap gClientMap;
pthread_mutex_t gClientMapMtx = PTHREAD_MUTEX_INITIALIZER;


ClientMap::ClientMap(int serverId, const string &serverInfo) {
	split(serverInfo, '\\');
	serverId_ = serverId;
	serverKey_ = NULL;
}


ClientMap::~ClientMap() {
	info_.clear();
	if(serverKey_)
		delete serverKey_;
	con_ = NULL;
	serverId_ = 0;
}


void ClientMap::Init() {}


void ClientMap::Dispose() {
	for(ClientMapMapIterator it = gClientMap.begin(); it != gClientMap.end(); it++) {
		it->first->Close();
		delete it->second;
	}
}


void ClientMap::InsertInfo(Conn *con, int serverId, const string &serverInfo) {
	pthread_mutex_lock( &gClientMapMtx );

	if(gClientMap.find(con) == gClientMap.end())
		gClientMap[con] = new ClientMap(serverId, serverInfo);

	pthread_mutex_unlock( &gClientMapMtx );
}


void ClientMap::DeleteInfo(Conn *con) {
	pthread_mutex_lock( &gClientMapMtx );

	if(gClientMap.find(con) != gClientMap.end())
		gClientMap.erase(con);

	pthread_mutex_unlock( &gClientMapMtx );
}


void ClientMap::PrintList(Console *con) {
	pthread_mutex_lock( &gClientMapMtx );

	con->PrintInfo("ClientMap\n");
	con->PrintInfo("-----------------------------------------------\n");
	for(ClientMapMapIterator it = gClientMap.begin(); it != gClientMap.end(); it++) {
		con->PrintInfo("  %i). %s -> %s\n", it->second->GetServerId(), it->first->RemoteIpAdress(), it->second->GetServerInfo(string("sv_hostname")).c_str());
	}

	pthread_mutex_unlock( &gClientMapMtx );
}

ClientMap *ClientMap::GetClientMap(Conn *con) {
	ClientMap *ret = NULL;

	pthread_mutex_lock( &gClientMapMtx );
	
	if(gClientMap.find(con) != gClientMap.end())
		ret = gClientMap[con];
	
	pthread_mutex_unlock( &gClientMapMtx );
	return ret;
}
string ClientMap::GetServerInfo(string key) {
	if(info_.find(key) != info_.end())
		return info_[key];

	return string("");
}


const string *ClientMap::GetServerKey() {
	return new string("");
}


int ClientMap::GetServerId() {
	return serverId_;
}


vector<string> &ClientMap::split(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}

	return elems;
}


void ClientMap::split(const string &s, char delim) {
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
