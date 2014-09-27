// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Q3dfApiImpl.h"
#include "RecordsystemDaemon.h"

#include <boost/program_options.hpp>
#include <sstream>
namespace po = boost::program_options;

const Error Q3dfApiImpl::ClientConnected(const ClientInfoRequest* args, NullResponse* reply) {
	//printf("clientConnected: pl=%i\n", args->identifier().playernum());
	return Error::Nil();
}


const Error Q3dfApiImpl::ClientDisconnected(const ClientInfoRequest* args, NullResponse* reply) {
	//printf("clientDisconnected: pl=%i\n", args->identifier().playernum());
	return Error::Nil();
}


const Error Q3dfApiImpl::ClientCommand(const ClientCommandRequest* args, ClientCommandResponse* reply) {
	Conn *con = static_cast<Conn *>(args->TagObj);

	vector<string> list;
	RepeatedPtrField<string>::const_iterator it;
	for(it = args->args().begin(); it != args->args().end(); ++it) {
		list.push_back(*it);
	}
	
	reply->mutable_identifier()->set_playernum(args->identifier().playernum());
	reply->mutable_identifier()->set_serverid(args->identifier().serverid());

	string output("");

	if(RS->HandleCommand(args->command(), &list, con, &output)) {
		reply->set_messagetoprint(output.c_str());
	}
	
	return Error::Nil();
}


const Error Q3dfApiImpl::Printf(const PrintfRequest* args, NullResponse* reply) {
	RS->Con()->Print(va("CL(%s): %s", ((Conn *)args->TagObj)->RemoteIpAdress(), args->msg().c_str()));
	return Error::Nil();
}


const Error Q3dfApiImpl::Login(const LoginRequest* args, LoginResponse* reply) {
	reply->mutable_identifier()->set_playernum(args->identifier().playernum());
	reply->mutable_identifier()->set_serverid(args->identifier().serverid());
	reply->set_hash("TEST");
	reply->set_userid(11);
	reply->set_result(LoginResponse_LoginResult_PASSED);
	return Error::Nil();
}


const Error Q3dfApiImpl::CheckForUpdates(const UpdateRequest* request, UpdateResponse* response) {
	string version("");
	string clientVersionKey("client_version");
	RS->Con()->Print("UpdateInfo with version %s\n", request->version().c_str());
	ifstream myfile (va("current_client_version%s", LIBRARY_EXT), ios::in|ios::binary|ios::ate);
	if(myfile.is_open()) {
		if(RS->Settings().find(clientVersionKey) != RS->Settings().end() && !RS->Settings()[clientVersionKey].empty()) {
			response->set_version(RS->Settings()[clientVersionKey]);
			response->set_available(true);
			streampos size = myfile.tellg();
			char *data = new char[size];
			myfile.seekg(0, ios::beg);
			myfile.read(data, size);
			myfile.close();
			response->set_data(data, size);
			delete data;
		}else
			response->set_available(false);
	}else
		response->set_available(false);

	return Error::Nil();
}


const Error Q3dfApiImpl::Register(const ServerRegisterRequest* request, NullResponse* response) {
	RS->Con()->Print("SERVERINFO: %s\n", request->serverinfostring().c_str());
	Conn *con = (Conn *)request->TagObj;

	RS->Clients()->Insert(con, request->serverid(), request->serverinfostring());

	return Error::Nil();
}


const Error Q3dfApiImpl::SaveRecord(const RecordRequest* request, NullResponse* response) {
	Conn *con = (Conn *)request->TagObj;

	RS->Con()->PrintInfo(
		va("RECORD %s %s %i\n",
			request->mapname().c_str(),
			request->name().c_str(),
			request->mstime()
		)
	);

	return Error::Nil();
}
