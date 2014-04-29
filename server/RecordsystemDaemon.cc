#include "RecordsystemDaemon.h"

#include <q3df_api.pb.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>
#include <cstdarg>
#include <pthread.h>
#include "Console.h"

#ifdef WIN32
#	include "ConsoleWin32.h"
#	include <WinSock.h>
#else
#	include <unistd.h>
#	include "ConsoleTty.h"
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	define Sleep usleep
#endif

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
private:
	Console *con;

public:
	inline Q3dfApi(Console *con) : con(con) {}
	virtual ~Q3dfApi() {}

	virtual const ::google::protobuf::rpc::Error ClientConnected(const ::service::ClientInfoRequest* args, ::service::NullResponse* reply) {
		//printf("clientConnected: pl=%i\n", args->identifier().playernum());
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error ClientDisconnected(const ::service::ClientInfoRequest* args, ::service::NullResponse* reply) {
		//printf("clientDisconnected: pl=%i\n", args->identifier().playernum());
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error ClientCommand(const ::service::ClientCommandRequest* args, ::service::ClientCommandResponse* reply) {
		reply->mutable_identifier()->set_playernum(args->identifier().playernum());
		reply->mutable_identifier()->set_uniqueid(args->identifier().uniqueid());

		reply->set_messagetoprint(va("ERROR: command '%s' not implemented bla", args->command().c_str()));
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error Printf(const ::service::PrintfRequest* args, ::service::NullResponse* reply) {
		this->con->Print(va("[Q3df]: %s", args->msg().c_str()));
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error Login(const ::service::LoginRequest* args, ::service::LoginResponse* reply) {
		reply->mutable_identifier()->set_playernum(args->identifier().playernum());
		reply->mutable_identifier()->set_uniqueid(args->identifier().uniqueid());
		reply->set_hash("TEST");
		reply->set_userid(11);
		reply->set_result(::service::LoginResponse_LoginResult_PASSED);
		return ::google::protobuf::rpc::Error::Nil();
	}

	virtual const ::google::protobuf::rpc::Error SaveRecord(const ::service::RecordRequest* request, ::service::NullResponse* response) {
		this->con->Print(va("RECORD: %s %s %i\n", request->mapname().c_str(), request->name().c_str(), request->mstime()));
		return ::google::protobuf::rpc::Error::Nil();
	}
};


int main(int argc, char **argv) {
	struct sockaddr_in addr;

#ifdef WIN32
	Console *con = new ConsoleWin32();
#else
	Console *con = new ConsoleTty();
#endif

	::google::protobuf::rpc::Server server(::google::protobuf::rpc::Env::Default());
	server.AddService(new Q3dfApi(con), true);
	server.ListenTCP(1234);

	for(;;) {
		char *cmd = con->Input();
		if(cmd && !strncmp(cmd, "exit", 4)) {
			break;
		}
		memset(&addr, 0, sizeof(addr));

		::google::protobuf::rpc::Conn *conn = server.AcceptNonBlock((sockaddr*)&addr);
		if(conn) {
			con->Print(va("Incoming connection from %s\n", inet_ntoa(addr.sin_addr)));
			server.Serve(conn);
		}

		Sleep(10);
	}

	con->Print(va("Shutingdown now...\n"));
	Sleep(1000);

	delete con;
	return 0;
}
