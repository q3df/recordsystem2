// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Q3dfApiImpl.h"


const Error Q3dfApiImpl::ClientConnected(const ClientInfoRequest* args, NullResponse* reply) {
	//printf("clientConnected: pl=%i\n", args->identifier().playernum());
	return Error::Nil();
}


const Error Q3dfApiImpl::ClientDisconnected(const ClientInfoRequest* args, NullResponse* reply) {
	//printf("clientDisconnected: pl=%i\n", args->identifier().playernum());
	return Error::Nil();
}


const Error Q3dfApiImpl::ClientCommand(const ClientCommandRequest* args, ClientCommandResponse* reply) {
	reply->mutable_identifier()->set_playernum(args->identifier().playernum());
	reply->mutable_identifier()->set_serverid(args->identifier().serverid());

	Conn *con = (Conn *)args->TagObj;
	string responseData("");


	if(args->command() == string("!top")) {
		sql::SQLString map(ClientMap::GetClientMap(con)->GetServerInfo(string("mapname")).c_str());
		sql::SQLString mode(ClientMap::GetClientMap(con)->GetServerInfo(string("defrag_mode")).c_str());
		sql::SQLString physic(ClientMap::GetClientMap(con)->GetServerInfo(string("df_promode")).c_str());
		
		sql::Connection *con = gMysqlPool->Get();

		try {
			int i = 0;
			std::auto_ptr< sql::PreparedStatement > stmt(con->prepareStatement("SELECT * FROM q3_defrag_records WHERE map = ? AND mode = ? AND physic = ? ORDER BY mstime LIMIT 10"));
			stmt->setString(1, map);
			stmt->setString(2, mode);
			stmt->setString(3, physic);

			std::auto_ptr< sql::ResultSet > res(stmt->executeQuery());
			while (res->next()) {
				i++;
				responseData.append(va(" %-3i). %-20s^7 %i\n", i, res->getString("nickname").c_str(), res->getInt("mstime")));
			}
		} catch (sql::SQLException &e) {
			responseData.clear();
			responseData.append(va("REMOTE ERROR: Q3dfApiImpl::ClientCommand::SqlError: '%s'\n", e.what()));
			gConsole->PrintError("Q3dfApiImpl::ClientCommand::SqlError: '%s'\n", e.what());
		}

		gMysqlPool->Return(con);
		reply->set_messagetoprint(responseData);
	}else{
		reply->set_messagetoprint(va("command '%s' not implemented!", args->command().c_str()));
	}
	
	return Error::Nil();
}


const Error Q3dfApiImpl::Printf(const PrintfRequest* args, NullResponse* reply) {
	this->con->Print(va("CL(%s): %s", ((Conn *)args->TagObj)->RemoteIpAdress(), args->msg().c_str()));
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
	this->con->Print("UpdateInfo with version %s\n", request->version().c_str());
	ifstream myfile (va("current_client_version%s", LIBRARY_EXT), ios::in|ios::binary|ios::ate);
	if(myfile.is_open()) {
		if(gSettings.find(clientVersionKey) != gSettings.end() && !gSettings[clientVersionKey].empty()) {
			response->set_version(gSettings[clientVersionKey]);
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
	this->con->Print("SERVERINFO: %s\n", request->serverinfostring().c_str());
	Conn *con = (Conn *)request->TagObj;

	ClientMap::InsertInfo(con, request->serverid(), request->serverinfostring());

	return Error::Nil();
}


const Error Q3dfApiImpl::SaveRecord(const RecordRequest* request, NullResponse* response) {
	Conn *con = (Conn *)request->TagObj;
	string responseData("");

	this->con->PrintInfo(
		va("RECORD %s %s %i\n",
			request->mapname().c_str(),
			request->name().c_str(),
			request->mstime()
		)
	);

	sql::Connection *mcon = gMysqlPool->Get();

	try {
		int i = 0;
		std::auto_ptr< sql::PreparedStatement > stmt(mcon->prepareStatement("SELECT * FROM q3_defrag_records WHERE map = ? AND mode = ? AND physic = ? ORDER BY mstime LIMIT 10"));
		stmt->setString(1, request->mapname().c_str());
		stmt->setInt(2, request->df_mode());
		stmt->setInt(3, request->df_promode());

		std::auto_ptr< sql::ResultSet > res(stmt->executeQuery());
		while (res->next()) {
			i++;
			responseData.append(va(" %-3i). %-20s^7 %i\n", i, res->getString("nickname").c_str(), res->getInt("mstime")));
		}
	} catch (sql::SQLException &e) {
		responseData.clear();
		responseData.append(va("REMOTE ERROR: Q3dfApiImpl::ClientCommand::SqlError: '%s'\n", e.what()));
		gConsole->PrintError("Q3dfApiImpl::ClientCommand::SqlError: '%s'\n", e.what());
	}
	gMysqlPool->Return(mcon);

	return Error::Nil();
}
