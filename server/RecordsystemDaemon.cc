#include "Q3dfEnv.h"
#include "RecordsystemDaemon.h"
#include "Q3dfApiImpl.h"
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

Console *gConsole;
Env *gEnvQ3df;
ProtobufOnceType gEnvQ3dfInitOnce;

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

int main(int argc, char **argv) {
#ifdef WIN32
	gConsole = new ConsoleWin32();
#else
	gConsole = new ConsoleTty();
#endif

	GoogleOnceInit(&gEnvQ3dfInitOnce, InitQ3dfEnv);

	Server server(gEnvQ3df);
	server.AddService(new Q3dfApiImpl(gConsole), true);
	server.ListenTCP(1234);

	for(;;) {
		char *cmd = gConsole->Input();
		if(cmd && !strncmp(cmd, "exit", 4)) {
			break;
		} else if(cmd && !strncmp(cmd, "status", 5)) {
			gConsole->Print("connected clients:\n");
			gConsole->Print("^3------------------------------\n");
			gConsole->PrintInfo("huhuhuhuhuhuhuhuhuh\n");
			gConsole->PrintError("could not read from file xyz...\n");
		}


		Conn *conn = server.AcceptNonBlock();
		if(conn) {
			gConsole->Print(va("^7[^3Q3df^7]: Incoming connection from %s\n", conn->RemoteIpAdress()));
			server.Serve(conn);
		}

		Sleep(10);
	}

	gConsole->PrintInfo(va("Shutingdown now...\n"));
	Sleep(1000);

	delete gConsole;
	return 0;
}
