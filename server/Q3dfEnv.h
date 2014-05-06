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

public:
	Q3dfEnv(Console *con) { con_ = con; }
	~Q3dfEnv() { exit(1); }

	// Write an entry to the log file with the specified format.
	virtual void Logv(const char* format, va_list ap) {
		const size_t kBufSize = 4096;
		char buffer[kBufSize+1];
		int written = vsnprintf(buffer, kBufSize, format, ap);
		buffer[kBufSize] = '\0';
		con_->Print(va("^1ERROR^7 Q3dfEnv: %s", buffer));
	}

	virtual void ClientDisconnect(Conn *con) {
		con_->Print(va("^3INFO^7 Q3dfEnv: client %s disconnected\n", con->RemoteIpAdress()));
	}
};

#endif // SERVER_Q3DFENV_H_