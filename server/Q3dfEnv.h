#ifndef SERVER_Q3DFENV_H_
#define SERVER_Q3DFENV_H_

#include "RecordsystemDaemon.h"
#include "Console.h"
#include <q3df_api.pb.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

void InitQ3dfEnv();

class Q3dfEnv : public Env {
private:
	Console *con_;
	void (*callbackDisconnect_)(Conn *con);

public:
	Q3dfEnv(Console *con) { con_ = con; callbackDisconnect_ = NULL; }
	~Q3dfEnv() { exit(1); }

	// Write an entry to the log file with the specified format.
	virtual void Logv(const char* format, va_list ap) {
		const size_t kBufSize = 4096;
		char buffer[kBufSize+1];
		int written = vsnprintf(buffer, kBufSize, format, ap);
		buffer[kBufSize] = '\0';
		con_->PrintError("Q3dfEnv: %s", buffer);
	}

	virtual void SetDisconnectCallback(void (*callbackDisconnect)(Conn *con)) {
		this->callbackDisconnect_ = callbackDisconnect;
	}

	virtual void ClientDisconnect(Conn *con) {
		con_->PrintInfo("Q3dfEnv: client %s disconnected\n", con->RemoteIpAdress());
		if(callbackDisconnect_ != NULL) callbackDisconnect_(con);
	}

	virtual bool Handshake(Conn *con) {
		string data("");
		string key("apikey");

		if(con->RecvFrame(&data) && gSettings.find(key) != gSettings.end() && gSettings[key] == data) {
			data.clear();
			data.append("OK");
			if(con->SendFrame(&data))
				return true;
		}else{
			data.clear();
			data.append("ACCESS DENIED!");
			con->SendFrame(&data);
		}

		return false;
	}
};

#endif // SERVER_Q3DFENV_H_