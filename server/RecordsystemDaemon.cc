#include "RecordsystemDaemon.h"

#include <Q3dfApi.pb.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>
#include <Windows.h>

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
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error Printf(const ::service::PrintfRequest* args, ::service::NullResponse* reply) {
		printf("[Q3df]: %s", args->msg().c_str());
		return ::google::protobuf::rpc::Error::Nil();
	}
};

int main(int argc, char **argv) {
	::google::protobuf::rpc::Server server;
	server.AddService(new Q3dfApi(), true);
	server.BindAndServe(1234);

	return 0;
}