#ifndef SERVER_Q3ENV_H_
#define SERVER_Q3ENV_H_

#include "Recordsystem.h"
#include <q3df_api.pb.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

void InitQ3Env();

class Q3Env : public Env {
public:
	Q3Env() {}
	~Q3Env() {}

	// Write an entry to the log file with the specified format.
	virtual void Logv(const char* format, va_list ap) {
		const size_t kBufSize = 4096;
		char buffer[kBufSize+1];
		int written = vsnprintf(buffer, kBufSize, format, ap);
		buffer[kBufSize] = '\0';
		RS_Print(va("Q3Env: %s", buffer));
	}

	virtual void ClientDisconnect(Conn *con) {
		RS_Print(va("Q3Env: client %s disconnected\n", con->RemoteIpAdress()));
	}

	virtual bool Handshake(Conn *con) {
		string res("");
		string apikey(rs_api_key.string);
		string serverId(rs_server_id.string);

		if(con->SendFrame(&(serverId + "_" + apikey))) {
			res.clear();
			if(con->RecvFrame(&res) && res == "OK")
				return true;
		}

		return false;
	}
};

extern Q3Env *gQ3Env;

#endif // SERVER_Q3ENV_H_
