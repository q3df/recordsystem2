#include "RconClient.h"

int InitSockets() {
#ifdef WIN32
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2,0),&wsa);
#else
	return 1;
#endif
}


RconClient::RconClient(const string &ip, int port, const string &password) {
	InitSockets();
	if ((sock_=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
		throw exception("socket initalizing faild!");
	}

	memset(&si_other_, 0, sizeof(si_other_));
    si_other_.sin_family = AF_INET;
    si_other_.sin_port = htons(port);
	si_other_.sin_addr.s_addr = inet_addr(ip.c_str());
	password_ = password;
}


RconClient::~RconClient() {
	
}

#define BUFFER_LEN 1024

const string &RconClient::SendCommand(const string &cmd) {
	static char retData[BUFFER_LEN];
	int len = 0;

	string *data = new string();
	data->append(gRconDataHeader);
	data->append(password_);
	data->append(" ");
	data->append(cmd);

	while(len < data->length()) {
		len += sendto(sock_, data->c_str()+len, data->length()-len, 0, (SOCKADDR *)&si_other_, sizeof(si_other_));
	}

	delete data;

	return string("TEST");
}
