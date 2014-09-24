// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_Q3DFENV_H_
#define SERVER_Q3DFENV_H_

#include <functional>
#include <q3df_api.pb.h>

#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

using namespace google::protobuf;
using namespace rpc;
using namespace service;

typedef function<void(Conn *)> DisconnectCallbackFunction;

class Q3dfEnv : public Env {
public:
	Q3dfEnv() { }
	~Q3dfEnv() { }

	// Write an entry to the log file with the specified format.
	virtual void Logv(const char* format, va_list ap) override;
	virtual void ClientDisconnect(Conn *con) override;
	virtual bool Handshake(Conn *con) override;
};

#endif // SERVER_Q3DFENV_H_