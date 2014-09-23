// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_Q3DFAPIIMPLE_H_
#define SERVER_Q3DFAPIIMPLE_H_

#include <q3df_api.pb.h>
#include <iostream>
#include <fstream>

#ifdef WIN32
#	define LIBRARY_EXT ".dll"
#else
#	define LIBRARY_EXT ".so"
#endif

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

class Q3dfApiImpl : public service::Q3dfApi {
public:
	Q3dfApiImpl() {}
	~Q3dfApiImpl() {}

	virtual const Error ClientConnected(const ClientInfoRequest* args, NullResponse* reply);
	virtual const Error ClientDisconnected(const ClientInfoRequest* args, NullResponse* reply);
	virtual const Error ClientCommand(const ClientCommandRequest* args, ClientCommandResponse* reply);
	virtual const Error Printf(const PrintfRequest* args, NullResponse* reply);
	virtual const Error Login(const LoginRequest* args, LoginResponse* reply);
	virtual const Error CheckForUpdates(const UpdateRequest* request, UpdateResponse* response);
	virtual const Error Register(const ServerRegisterRequest* request, NullResponse* response);
	virtual const Error SaveRecord(const RecordRequest* request, NullResponse* response);
};

#endif // SERVER_Q3DFAPIIMPLE_H_
