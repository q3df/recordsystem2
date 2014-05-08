#include <stdio.h>
#include "Plugin.h"

class UserCommandsPlugin : public PluginBase {
private:
	void OnGameClientCommand(Q3EventArgs *e);

public:
	virtual void Init();
	virtual void Destroy();
	virtual const char *Name() { return "UserCommandsPlugin"; }
};

void UserCommandsPlugin::Init() {
	RS_BEFORE_AddEventHandler(GAME_CLIENT_COMMAND, this, UserCommandsPlugin::OnGameClientCommand);
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

		EXECUTE_API_ASYNC(&Q3dfApi_Stub::ClientCommand, cmdReq, cmdRes, [](Message *msg, rpc::Error *error) {
			ClientCommandResponse *res = (ClientCommandResponse *)msg;

			if(!error->IsNil()) {
				RS_Syscall->SendServerCommand(0, va("print \"ERROR: %s\n\"", error->String().c_str()));
				return;
			}else{
				RS_Syscall->SendServerCommand(res->identifier().playernum(), va("print \"%s\n\"", res->messagetoprint().c_str()));
			}
		});
	}
}

// Register plugin!
RegisterPlugin(UserCommandsPlugin);
