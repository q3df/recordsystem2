#include "RecordsystemDaemon.h"

#include <Q3dfApi.pb.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>
#include <cstdarg>

extern "C" {
	const char *va( const char *format, ... ) {
		va_list		argptr;
		static char		string[2][32000];	// in case va is called by nested functions
		static int		index = 0;
		char	*buf;

		buf = string[index & 1];
		index++;

		va_start (argptr, format);
		vsprintf (buf, format,argptr);
		va_end (argptr);

		return buf;
	}
}

class Q3dfApi: public service::Q3dfApi {
public:
	inline Q3dfApi() {}
	virtual ~Q3dfApi() {}

	virtual const ::google::protobuf::rpc::Error ClientConnected(const ::service::ClientInfoRequest* args, ::service::NullResponse* reply) {
		printf("clientConnected: pl=%i\n", args->playernum());
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error ClientDisconnected(const ::service::ClientInfoRequest* args, ::service::NullResponse* reply) {
		printf("clientDisconnected: pl=%i\n", args->playernum());
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error ClientCommand(const ::service::ClientCommandRequest* args, ::service::ClientCommandResponse* reply) {
		reply->set_playernum(args->playernum());
		reply->set_messagetoprint(va("ERROR: command '%s' not implemented", args->command().c_str()));
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error Printf(const ::service::PrintfRequest* args, ::service::NullResponse* reply) {
		printf("[Q3df]: %s", args->msg().c_str());
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error Login(const ::service::LoginRequest* args, ::service::NullResponse* reply) {
		return ::google::protobuf::rpc::Error("not implemented!");
	}
};

int main(int argc, char **argv) {
	::google::protobuf::rpc::Server server;
	server.AddService(new Q3dfApi(), true);
	server.BindAndServe(1234);

	return 0;
}
