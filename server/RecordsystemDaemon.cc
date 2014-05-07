#include "Q3dfEnv.h"
#include "RecordsystemDaemon.h"
#include "Q3dfApiImpl.h"
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>
#include <list>

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

std::list<Conn *> clientList;

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

void clientDisconnected(Conn *con) {
	clientList.remove(con);
}

int main(int argc, char **argv) {
#ifdef WIN32
	ConsoleWin32 *con = new ConsoleWin32();
	gConsole = con;
#else
	ConsoleTty *con = new ConsoleTty();
	gConsole = con;
#endif

	GoogleOnceInit(&gEnvQ3dfInitOnce, InitQ3dfEnv);

	((Q3dfEnv *)gEnvQ3df)->SetDisconnectCallback(clientDisconnected);

	Server server(gEnvQ3df);
	server.AddService(new Q3dfApiImpl(gConsole), true);
	server.ListenTCP(1234);

	for(;;) {
		char *cmd = gConsole->Input();
		if(cmd && !strncmp(cmd, "exit", 4)) {
			break;
		} else if(cmd && !strncmp(cmd, "status", 5)) {
			gConsole->Print(va("%i client(s) connected:\n", clientList.size()));
			gConsole->Print("^3------------------------------^7\n");
			for (std::list<Conn *>::iterator it=clientList.begin(); it != clientList.end(); ++it)
				gConsole->Print(va("    * %s\n", (*it)->RemoteIpAdress()));
		}


		Conn *conn = server.AcceptNonBlock();
		if(conn) {
			clientList.push_back(conn);
			gConsole->Print(va("^7[^3Q3df^7]: Incoming connection from %s\n", conn->RemoteIpAdress()));
			server.Serve(conn);
			
		}

		Sleep(10);
	}

	gConsole->PrintInfo(va("Shutingdown now...\n"));
	Sleep(1000);

	delete con;
	return 0;
}
