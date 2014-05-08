#include <stdio.h>
#include "Plugin.h"
#include <functional>

class UserManagementPlugin : public PluginBase {
private:
	vmCvar_t cvarRsNoNewClients;
	vmCvar_t cvarRsWelcome1;
	vmCvar_t cvarRsWelcome2;
	vmCvar_t cvarRsWelcome3;

	void OnClientConnected(Q3EventArgs *e);
	void OnClientDisconnected(Q3EventArgs *e);
	void OnClientUserInfoChanged(Q3EventArgs *e);
	void OnClientBegin(Q3EventArgs *e);

	void OnSyscallGetUserInfo(Q3EventArgs *e);
	void OnSyscallSetUserInfo(Q3EventArgs *e);

	void OnVmMainShutdown(Q3EventArgs *e);

public:
	virtual void Init();
	virtual void Destroy();
	virtual const char *Name() { return "UserManagementPlugin"; }
};

void UserManagementPlugin::Init() {
	RS_BEFORE_AddEventHandler(GAME_CLIENT_CONNECT, this, UserManagementPlugin::OnClientConnected);
	RS_BEFORE_AddEventHandler(GAME_CLIENT_DISCONNECT, this, UserManagementPlugin::OnClientDisconnected);
	RS_BEFORE_AddEventHandler(GAME_CLIENT_USERINFO_CHANGED, this, UserManagementPlugin::OnClientUserInfoChanged);
	RS_BEFORE_AddEventHandler(GAME_CLIENT_BEGIN, this, UserManagementPlugin::OnClientBegin);
	RS_BEFORE_AddEventHandler(G_GET_USERINFO, this, UserManagementPlugin::OnSyscallGetUserInfo);
	RS_BEFORE_AddEventHandler(G_SET_USERINFO, this, UserManagementPlugin::OnSyscallSetUserInfo);
	RS_BEFORE_AddEventHandler(GAME_SHUTDOWN, this, UserManagementPlugin::OnVmMainShutdown);

	gRecordsystem->RegisterCvar(&cvarRsNoNewClients, "rs_no_new_clients", "0", 0, qfalse);
	gRecordsystem->RegisterCvar(&cvarRsWelcome1, "rs_welcome1", "q3df - recordsystem - v0.1 (alpha)", 0, qfalse);
	gRecordsystem->RegisterCvar(&cvarRsWelcome2, "rs_welcome2", "", 0, qfalse);
	gRecordsystem->RegisterCvar(&cvarRsWelcome3, "rs_welcome3", "", 0, qfalse);
}

void UserManagementPlugin::Destroy() {

}


/*******************************************************************************
 * Eventhandler's
 */
void UserManagementPlugin::OnClientConnected(Q3EventArgs *e) {
	Q3User *cl = NULL;
	ClientInfoRequest *clReq = NULL;
	NullResponse *nullRes = NULL;

	int playernum = e->GetParam(0);
	bool isFirstTime = e->GetParam(1) == 1 ? true : false;

	if(cvarRsNoNewClients.integer == 1) {
		e->SetReturn((void *)"Sorry, server in latched for shutdown.");
		return;
	}

	cl = gRecordsystem->GetUser(playernum);
	cl->Reset();
	cl->SetUniqueId(RS_Syscall->Milliseconds());

	OnClientUserInfoChanged(e); // we can forward because param 1 is our playernum!

	if(isFirstTime) {
		cl->SetState(CLIENT_CONNECTED);
		RS_Print(va("client:%i connecting (%s)...\n", cl->GetPlayernum(), cl->GetUserInfo("name")));

		nullRes = new NullResponse();
		clReq = new ClientInfoRequest();
		cl->WriteIdentifier(clReq->mutable_identifier());
		clReq->set_userid(0);
		clReq->set_userinfostring(cl->GetUserInfoString());

		EXECUTE_API_ASYNC(&Q3dfApi_Stub::ClientConnected, clReq, nullRes, NULL);
	} else {
		// restore client
		cl->SetState(CLIENT_ACTIVE);
		cl->SetUserId(RS_Syscall->CvarVariableIntegerValue(va("loginstate%i", playernum)));
		RS_Print(va("client:%i restoring (%s)...\n", cl->GetPlayernum(), cl->GetUserInfo("name")));
	}
}


void UserManagementPlugin::OnClientDisconnected(Q3EventArgs *e) {
	NullResponse *nullRes = new NullResponse();
	ClientInfoRequest *clReq = new ClientInfoRequest();

	int playernum = e->GetParam(0);
	Q3User *cl = gRecordsystem->GetUser(playernum);

	cl->WriteIdentifier(clReq->mutable_identifier());
	clReq->set_userid(cl->GetUserId());
	clReq->set_userinfostring(cl->GetUserInfoString());

	EXECUTE_API_ASYNC(&Q3dfApi_Stub::ClientDisconnected, clReq, nullRes, NULL);
	
	cl->Reset();

	RS_Syscall->CvarSet(va("loginstate%i", playernum), "0");
}


void UserManagementPlugin::OnClientUserInfoChanged(Q3EventArgs *e) {
	static char tmpBuffer[BIG_INFO_STRING];
	Q3User *cl = NULL;
	LoginRequest *lReq = NULL;
	LoginResponse *lRes = NULL;

	int playernum = e->GetParam(0);

	RS_Syscall->GetUserinfo(playernum, tmpBuffer, sizeof(tmpBuffer));

	cl = gRecordsystem->GetUser(playernum);
	cl->SetUserInfoString(tmpBuffer); // will be copied!

	const char *q3dfkey = cl->GetUserInfo("q3dfkey");
	if((!q3dfkey || strlen(q3dfkey) <= 0) && cl->GetUserId() != 0) {
		cl->SetUserId(0);
		RS_Syscall->SendServerCommand(cl->GetPlayernum(), "print \"^7[^3Q3df::Info^7] you're currently logged out...\n\"");
	}else if(cl->GetUserId() == 0 || strcmp(cl->GetLastQ3dfkey(), q3dfkey)) {
		lReq = new LoginRequest();
		cl->WriteIdentifier(lReq->mutable_identifier());
		lReq->set_q3dfkey(q3dfkey);

		lRes = new LoginResponse();

		EXECUTE_API_ASYNC(&Q3dfApi_Stub::Login, lReq, lRes, [](Message *msg, rpc::Error *error) {
			LoginResponse *res = (LoginResponse *)msg;

			if(!error->IsNil()) {
				RS_Syscall->SendServerCommand(
					res->identifier().playernum(), 
					va("print \"^7[^1Q3df::Error^7] %s\n\"", error->String().c_str())
				);
				RS_Syscall->SendServerCommand(
					res->identifier().playernum(), 
					"print \"^7[^3Q3df::Info^7] you're currently ^1NOT^7 logged in!\n\""
				);
				gRecordsystem->GetUser(res->identifier().playernum())->SetUserId(0);
				return;
			}else{
				RS_Syscall->SendServerCommand(
					res->identifier().playernum(), 
					va("print \"^7[^3Q3df::Info^7] you're currently logged in with userid %i!\n\"", res->userid())
				);
				gRecordsystem->GetUser(res->identifier().playernum())->SetUserId(res->userid());
			}
		});
	}
}


void UserManagementPlugin::OnClientBegin(Q3EventArgs *e) {
	int playernum = e->GetParam(0);
	Q3User *cl = gRecordsystem->GetUser(playernum);

	cl->SetState(CLIENT_ACTIVE);

	if(strlen(cvarRsWelcome1.string) > 0)
		RS_Syscall->SendServerCommand(playernum, va("chat \"[Q3df] %s\"", cvarRsWelcome1.string));

	if(strlen(cvarRsWelcome2.string) > 0)
		RS_Syscall->SendServerCommand(playernum, va("chat \"[Q3df] %s\"", cvarRsWelcome2.string));

	if(strlen(cvarRsWelcome3.string) > 0)
		RS_Syscall->SendServerCommand(playernum, va("chat \"[Q3df] %s\"", cvarRsWelcome3.string));

	if(cl->GetUserId() > 0)
		RS_Syscall->SendServerCommand(playernum, va("print \"^7[^3Q3df::Info^7] you're currently logged in with userid ^3%i^7.\n\"", cl->GetUserId()));
	else
		RS_Syscall->SendServerCommand(playernum, va("print \"^7[^3Q3df::Info^7] you're ^1NOT^7 logged in!\n\"", cl->GetUserId()));
}


void UserManagementPlugin::OnSyscallGetUserInfo(Q3EventArgs *e) {
	int playernum = e->GetParam(0);
	int bufferSize = e->GetParam(2);
	char *buffer = (char *)e->GetParamVMA(1);

	Q3User *cl = gRecordsystem->GetUser(playernum);

	strncpy(buffer, cl->GetUserInfoString(), bufferSize);
	e->SetHandled(true);
}


void UserManagementPlugin::OnSyscallSetUserInfo(Q3EventArgs *e) {
	static char tmpBuffer[BIG_INFO_STRING];
	int playernum = e->GetParam(0);
	const char *buffer = (const char *)e->GetParamVMA(1);

	strncpy(tmpBuffer, buffer, sizeof(tmpBuffer));
	gRecordsystem->GetUser(playernum)->SetUserInfoString(tmpBuffer);
}


void UserManagementPlugin::OnVmMainShutdown(Q3EventArgs *e) {
	Q3User *cl = NULL;
	int i = 0;

	for(i = 0; i<gRecordsystem->GetUserCount(); i++) {
		cl = gRecordsystem->GetUser(i);
		if(cl->GetState() != CLIENT_FREE)
			RS_Syscall->CvarSet(va("loginstate%i", i), va("%i", cl->GetUserId()));
	}
}

// Register plugin!
RegisterPlugin(UserManagementPlugin);
