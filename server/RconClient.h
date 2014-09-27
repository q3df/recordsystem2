#ifndef SERVER_RCONCLIENT_H__
#define SERVER_RCONCLIENT_H__

#include <stdio.h>
#include <sys/types.h>
#include <string>

#ifdef WIN32
#	include <Windows.h>
#else
#	include <arpa/inet.h>
#	include <netinet/in.h>
#	include <sys/socket.h>
#	include <unistd.h>
#endif

static char gRconDataHeader[] = { '\xFF', '\xFF', '\xFF', '\xFF', 'r', 'c', 'o', 'n', ' ', '\0' };
using namespace std;

class RconClient {
private:
	SOCKET sock_;
	struct sockaddr_in si_other_;
	string password_;

public:
	RconClient(const string &ip, int port, const string &password);
	~RconClient();

	void SendCommand(const string &cmd);
};

#endif // SERVER_RCONCLIENT_H__
