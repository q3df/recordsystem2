#include <stdio.h>
#include "Plugin.h"
#include <queue>
#include <sstream>
#include <string>

class UserCommandsPlugin : public PluginBase {
private:
	int counter_;
	queue<string> sentTextLinesToClient;
	void OnGameClientCommand(Q3EventArgs *e);
	void OnGameRunFrame(Q3EventArgs *e);
	void OnExecClientCommandFinished(Message *msg, rpc::Error *error);

public:
	virtual void Init();
	virtual void Destroy();
	virtual const char *Name() { return "UserCommandsPlugin"; }
};

void UserCommandsPlugin::Init() {
	counter_ = 0;
	RS_AFTER_AddEventHandler(GAME_RUN_FRAME, this, UserCommandsPlugin::OnGameRunFrame);
	RS_BEFORE_AddEventHandler(GAME_CLIENT_COMMAND, this, UserCommandsPlugin::OnGameClientCommand);
}


void UserCommandsPlugin::Destroy() {
}


void UserCommandsPlugin::OnGameRunFrame(Q3EventArgs *e) {
	int i = 0;

	if(counter_ % 4) {
		string printData("");
		while(i < 5 && !sentTextLinesToClient.empty()) {
			string item = sentTextLinesToClient.front();
			sentTextLinesToClient.pop();
			printData.append(item);
			printData.append("\n");
			i++;
		}

		if(printData.size() > 0 && sentTextLinesToClient.empty()) {
			RS_Syscall->SendServerCommand(0, va("print \"%s\n\"", printData.c_str()));
		}else if(printData.size() > 0 && !sentTextLinesToClient.empty()) {
			RS_Syscall->SendServerCommand(0, va("print \"%s\"", printData.c_str()));
		}
	}
	this->counter_++;
}


void UserCommandsPlugin::OnGameClientCommand(Q3EventArgs *e) {
	int i;
	char arg[255];
	bool isLoginCommand = false;
	bool isRsCommand = false;
	ClientCommandRequest *cmdReq = NULL;
	ClientCommandResponse *cmdRes = NULL;

	int playernum = e->GetParam(0);
	int argc = RS_Syscall->Argc();

	Q3User *cl = gRecordsystem->GetUser(playernum);


	RS_Syscall->Argv(0, arg, sizeof(arg));
	if(!strncmp(arg, "login", 5)) {
		isLoginCommand = true;
		e->SetHandled(true);
	}

	RS_Syscall->Argv(1, arg, sizeof(arg));
	if(!strncmp(arg, "!login", 6)) {
		isLoginCommand = true;
		e->SetHandled(true);
	}

	if(!strncmp(arg, "!", 1)) {
		isRsCommand = true;
		e->SetHandled(true);
	}

	if(isLoginCommand) {
		RS_Syscall->SendServerCommand(playernum, "print \"ERROR: !login is no longer supported. please read http://q3df.org/wiki?p=XX\n\"");
		return;
	}

	if(isRsCommand) {
		cmdReq = new ClientCommandRequest();
		cl->WriteIdentifier(cmdReq->mutable_identifier());
		cmdReq->set_command(arg);

		cmdRes = new ClientCommandResponse();

		for(i=2; i<argc; i++) {
			RS_Syscall->Argv(i, arg, sizeof(arg));

			std::string *tmpStr = cmdReq->add_args();
			tmpStr->append(arg);
		}

		auto callback = std::bind(&UserCommandsPlugin::OnExecClientCommandFinished, this, std::placeholders::_1, std::placeholders::_2);
		EXECUTE_API_ASYNC(&Q3dfApi_Stub::ClientCommand, cmdReq, cmdRes, callback);
	}
}

void UserCommandsPlugin::OnExecClientCommandFinished(Message *msg, rpc::Error *error) {
	ClientCommandResponse *res = (ClientCommandResponse *)msg;

	if(!error->IsNil()) {
		RS_Syscall->SendServerCommand(0, va("print \"^1ERROR:^7 %s\n\"", error->String().c_str()));
		return;
	}else{
		//RS_Syscall->SendServerCommand(res->identifier().playernum(), va("print \"%s\n\"", res->messagetoprint().c_str()));
		stringstream ss;
		string line;
		ss << res->messagetoprint();
		while(std::getline(ss, line, '\n')) {
			this->sentTextLinesToClient.push(line);
		}
	}
}

// Register plugin!
RegisterPlugin(UserCommandsPlugin);
