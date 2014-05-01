#include "RecordsystemDaemon.h"

#include <q3df_api.pb.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>
#include <cstdarg>
#include <pthread.h>
#include "Console.h"

#ifdef WIN32
#	include "ConsoleWin32.h"
#else
#	include <unistd.h>
#	include "ConsoleTty.h"
#	define Sleep usleep
#endif

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

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

Console *con;

static ProtobufOnceType g_env_q3df_init_once;
static Env* g_env_q3df;

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

static void InitQ3dfEnv() {
  g_env_q3df = new Q3dfEnv(con);
}

int main(int argc, char **argv) {
#ifdef WIN32
	con = new ConsoleWin32();
#else
	con = new ConsoleTty();
#endif

	GoogleOnceInit(&g_env_q3df_init_once, InitQ3dfEnv);

	Server server(g_env_q3df);
	server.AddService(new Q3dfApiImpl(con), true);
	server.ListenTCP(1234);

	for(;;) {
		char *cmd = con->Input();
		if(cmd && !strncmp(cmd, "exit", 4)) {
			break;
		}

		Conn *conn = server.AcceptNonBlock();
		if(conn) {
			con->Print(va("^7[^3Q3df^7]: Incoming connection from %s\n", conn->RemoteIpAdress()));
			server.Serve(conn);
		}

		Sleep(10);
	}

	con->Print(va("Shutingdown now...\n"));
	Sleep(1000);

	delete con;
	return 0;
}
