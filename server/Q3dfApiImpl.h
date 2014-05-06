#ifndef SERVER_Q3DFAPIIMPLE_H_
#define SERVER_Q3DFAPIIMPLE_H_

#include <q3df_api.pb.h>
#include "Console.h"
#include "RecordsystemDaemon.h"

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

class Q3dfApiImpl : public service::Q3dfApi {
private:
	Console *con;

public:
	inline Q3dfApiImpl(Console *con) : con(con) {}
	virtual ~Q3dfApiImpl() {}

	virtual const Error ClientConnected(const ClientInfoRequest* args, NullResponse* reply) {
		//printf("clientConnected: pl=%i\n", args->identifier().playernum());
		return Error::Nil();
	}

	virtual const Error ClientDisconnected(const ClientInfoRequest* args, NullResponse* reply) {
		//printf("clientDisconnected: pl=%i\n", args->identifier().playernum());
		return Error::Nil();
	}

	virtual const Error ClientCommand(const ClientCommandRequest* args, ClientCommandResponse* reply) {
		reply->mutable_identifier()->set_playernum(args->identifier().playernum());
		reply->mutable_identifier()->set_serverid(args->identifier().serverid());

		reply->set_messagetoprint(va("^7[^1Q3df::Error^7]: command '%s' not implemented bla", args->command().c_str()));
		return Error::Nil();
	}

	virtual const Error Printf(const PrintfRequest* args, NullResponse* reply) {
		this->con->Print(va("^7[^3Q3df^7]: %s", args->msg().c_str()));
		return Error::Nil();
	}

	virtual const Error Login(const LoginRequest* args, LoginResponse* reply) {
		reply->mutable_identifier()->set_playernum(args->identifier().playernum());
		reply->mutable_identifier()->set_serverid(args->identifier().serverid());
		reply->set_hash("TEST");
		reply->set_userid(11);
		reply->set_result(LoginResponse_LoginResult_PASSED);
		return Error::Nil();
	}

	virtual const Error SaveRecord(const RecordRequest* request, NullResponse* response) {
		this->con->Print(
			va("^7[^3Q3df::Info^7]: RECORD %s %s %i\n",
				request->mapname().c_str(),
				request->name().c_str(),
				request->mstime()
			)
		);
		return Error::Nil();
	}
};

#endif // SERVER_Q3DFAPIIMPLE_H_