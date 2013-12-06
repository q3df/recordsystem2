#include <stdio.h>
#include "Plugin.h"
#include "../Recordsystem.h"
#include "../Logger.h"

#include <Q3dfApi.pb.h>

using namespace google::protobuf;
using namespace service;

class UserCommandsPlugin : public PluginBase {
private:
	void OnGameClientCommand(Q3EventArgs *e);

public:
	virtual void Init();
	virtual void Destroy();
};

void UserCommandsPlugin::Init() {
	auto clientCommandEvent = std::bind(&UserCommandsPlugin::OnGameClientCommand, this, std::placeholders::_1);
	gRecordsystem->AddEventHandler(new Q3EventHandler(GAME_CLIENT_COMMAND, EXECUTE_TYPE_BEFORE, clientCommandEvent));
}

void UserCommandsPlugin::Destroy() {
}


void UserCommandsPlugin::OnGameClientCommand(Q3EventArgs *e) {
	int i;
	char arg[255];
	bool isLoginCommand = false;
	bool isRsCommand = false;
	ClientCommandRequest *cmdReq = NULL;
	ClientCommandResponse *cmdRes = NULL;

	int playernum = e->GetParam(0);
	int argc = gRecordsystem->GetSyscalls()->Argc();

	gRecordsystem->GetSyscalls()->Argv(0, arg, sizeof(arg));
	if(!strncmp(arg, "login", 5)) {
		isLoginCommand = true;
		e->SetHandled(true);
	}

	gRecordsystem->GetSyscalls()->Argv(1, arg, sizeof(arg));
	if(!strncmp(arg, "!login", 6)) {
		isLoginCommand = true;
		e->SetHandled(true);
	}

	if(!strncmp(arg, "!", 1)) {
		isRsCommand = true;
		e->SetHandled(true);
	}

	if(isLoginCommand) {
		gRecordsystem->GetSyscalls()->SendServerCommand(playernum, "print \"ERROR: !login is no longer supported. please read http://q3df.org/wiki?p=33\n\"");
		return;
	}

	if(isRsCommand) {
		cmdReq = new ClientCommandRequest();
		cmdReq->set_playernum(playernum);
		cmdReq->set_command(arg);

		cmdRes = new ClientCommandResponse();

		for(i=2; i<argc; i++) {
			gRecordsystem->GetSyscalls()->Argv(i, arg, sizeof(arg));

			std::string *tmpStr = cmdReq->add_args();
			tmpStr->append(arg);
		}

		EXECUTE_API_ASYNC(&Q3dfApi_Stub::ClientCommand, gRecordsystem->GetQ3dfApi(), cmdReq, cmdRes, [](Message *msg, rpc::Error *error) {
			ClientCommandResponse *res = (ClientCommandResponse *)msg;

			if(!error->IsNil()) {
				gRecordsystem->GetSyscalls()->SendServerCommand(0, va("print \"ERROR: %s\n\"", error->String().c_str()));
				return;
			}else{
				gRecordsystem->GetSyscalls()->SendServerCommand(res->playernum(), va("print \"%s\n\"", res->messagetoprint().c_str()));
			}
		});
	}
}

// Register plugin!
static PluginProxy<UserCommandsPlugin> gUserCommandsPlugin;
