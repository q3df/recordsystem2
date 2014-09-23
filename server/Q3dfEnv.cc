// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Q3dfEnv.h"
#include "RecordsystemDaemon.h"

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

void Q3dfEnv::Logv(const char* format, va_list ap) {
	const size_t kBufSize = 4096;
	char buffer[kBufSize+1];
	int written = vsnprintf(buffer, kBufSize, format, ap);
	buffer[kBufSize] = '\0';
	RS->Con()->PrintError("Q3dfEnv: %s", buffer);
}

void Q3dfEnv::ClientDisconnect(Conn *con) {
	RS->Con()->PrintInfo("client %s disconnected\n", con->RemoteIpAdress());
	RS->Clients()->Delete(con);
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
		   && RS->Settings().find(settingsKey) != RS->Settings().end()
		   && RS->Settings()[settingsKey] == apikey) {
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