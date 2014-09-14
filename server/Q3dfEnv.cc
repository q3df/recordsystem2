// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Q3dfEnv.h"
#include "ClientMap.h"

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

Q3dfEnv *gEnvQ3df;
ProtobufOnceType gEnvQ3dfInitOnce;


void InitQ3dfEnv() {
	gEnvQ3df = new Q3dfEnv(gConsole);
}


void Q3dfEnv::Init() {
	GoogleOnceInit(&gEnvQ3dfInitOnce, InitQ3dfEnv);

	gEnvQ3df->SetDisconnectCallback([](Conn *con) {
		ClientMap::DeleteInfo(con);
	});
}


void Q3dfEnv::Dispose() {
	delete gEnvQ3df;
	gEnvQ3df = NULL;
}


void Q3dfEnv::Logv(const char* format, va_list ap) {
	const size_t kBufSize = 4096;
	char buffer[kBufSize+1];
	int written = vsnprintf(buffer, kBufSize, format, ap);
	buffer[kBufSize] = '\0';
	con_->PrintError("Q3dfEnv: %s", buffer);
}


void Q3dfEnv::SetDisconnectCallback(DisconnectCallbackFunction callback) {
	this->callbackDisconnect_ = callback;
}


void Q3dfEnv::ClientDisconnect(Conn *con) {
	con_->PrintInfo("Q3dfEnv: client %s disconnected\n", con->RemoteIpAdress());
	callbackDisconnect_(con);
}


bool Q3dfEnv::Handshake(Conn *con) {
	string data("");

	string apikey("");
	string serverId("");


	if(con->RecvFrame(&data) && data.length() > 0 && data.find_first_of("_") > 0) {
		int pos = data.find_first_of("_");

		for(string::iterator it = data.begin(); it != data.end(); it++) {
			if((it-data.begin()) > pos)
				apikey.append(&it[0], 1);
			else if((it-data.begin()) < pos)
				serverId.append(&it[0], 1);
		}

		string settingsKey("apikey-"+serverId);
		if(apikey.length() > 0 && serverId.length() > 0
		   && gSettings.find(settingsKey) != gSettings.end()
		   && gSettings[settingsKey] == apikey) {
			data.clear();
			data.append("OK");
			if(con->SendFrame(&data))
				return true;
		}
	}

	data.clear();
	data.append("ACCESS DENIED!");
   	con->SendFrame(&data);
	return false;
}