// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_Q3DFENV_H_
#define SERVER_Q3DFENV_H_

#include <functional>
#include <q3df_api.pb.h>

#include "RecordsystemDaemon.h"
#include "Console.h"

#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

typedef std::function<void(google::protobuf::rpc::Conn*)> DisconnectCallbackFunction;

class Q3dfEnv : public Env {
private:
	Console *con_;
	DisconnectCallbackFunction callbackDisconnect_;

public:
	Q3dfEnv(Console *con) { con_ = con; callbackDisconnect_ = NULL; }
	~Q3dfEnv() { }

	static void Init();
	static void Dispose();

	// Write an entry to the log file with the specified format.
	virtual void Logv(const char* format, va_list ap);
	virtual void SetDisconnectCallback(DisconnectCallbackFunction callback);
	virtual void ClientDisconnect(Conn *con);
	virtual bool Handshake(Conn *con);
};

extern Q3dfEnv *gEnvQ3df;
extern ProtobufOnceType gEnvQ3dfInitOnce;

#endif // SERVER_Q3DFENV_H_