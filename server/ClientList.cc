// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientList.h"
#include <pthread.h>

ClientInfo::ClientInfo(Conn *con, int serverId, const string &serverInfo) {
	this->con_ = con;
	this->split(serverInfo, '\\');
	this->serverId_ = serverId;
}


ClientInfo::~ClientInfo() {
	info_.clear();
	con_ = NULL;
	serverId_ = 0;
}


ClientList::ClientList() {
	this->clientMapMtx_ = PTHREAD_MUTEX_INITIALIZER;
}


ClientList::~ClientList() {
	pthread_mutex_lock( &clientMapMtx_ );

	for(map<Conn*,ClientInfo*>::iterator it = clList_.begin(); it != clList_.end(); it++) {
		it->first->Close();
		delete it->second;
	}



	pthread_mutex_unlock( &clientMapMtx_ );
	pthread_mutex_destroy(&clientMapMtx_ );
}


void ClientList::Insert(Conn *con, int serverId, const string &serverInfo) {
	pthread_mutex_lock( &clientMapMtx_ );

	if(clList_.find(con) == clList_.end())
		clList_[con] = new ClientInfo(con, serverId, serverInfo);

	pthread_mutex_unlock( &clientMapMtx_ );
}


void ClientList::Delete(Conn *con) {
	pthread_mutex_lock( &clientMapMtx_ );

	if(clList_.find(con) != clList_.end())
		clList_.erase(con);

	pthread_mutex_unlock( &clientMapMtx_ );
}


//void ClientList::Print(Console *con) {
//	pthread_mutex_lock( &clientMapMtx_ );
//
//	con->PrintInfo("ClientMap\n");
//	con->PrintInfo("-----------------------------------------------\n");
//	for(ClientMapMapIterator it = clList_.begin(); it != clList_.end(); it++) {
//		con->PrintInfo("  %i). %s -> %s\n", it->second->GetServerId(), it->first->RemoteIpAdress(), it->second->GetServerInfo(string("sv_hostname")).c_str());
//	}
//
//	pthread_mutex_unlock( &clientMapMtx_ );
//}

ClientInfo *ClientList::GetClient(Conn *con) {
	ClientInfo *ret = NULL;

	pthread_mutex_lock( &clientMapMtx_ );
	
	if(clList_.find(con) != clList_.end())
		ret = clList_[con];
	
	pthread_mutex_unlock( &clientMapMtx_ );
	return ret;
}





string ClientInfo::GetServerInfo(string key) {
	if(info_.find(key) != info_.end())
		return info_[key];

	return string("");
}


int ClientInfo::GetServerId() {
	return serverId_;
}


vector<string> &ClientInfo::split(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}

	return elems;
}


void ClientInfo::split(const string &s, char delim) {
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
