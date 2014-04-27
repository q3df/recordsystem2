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
};

void UserManagementPlugin::Init() {

	auto clientConnectedHook = std::bind(&UserManagementPlugin::OnClientConnected, this, std::placeholders::_1);
	gRecordsystem->AddEventHandler(new Q3EventHandler(GAME_CLIENT_CONNECT, EXECUTE_TYPE_BEFORE, clientConnectedHook));
	
	auto clientDisconnectedHook = std::bind(&UserManagementPlugin::OnClientDisconnected, this, std::placeholders::_1);
	gRecordsystem->AddEventHandler(new Q3EventHandler(GAME_CLIENT_DISCONNECT, EXECUTE_TYPE_BEFORE, clientDisconnectedHook));
	
	auto clientUserInfoChangedHook = std::bind(&UserManagementPlugin::OnClientUserInfoChanged, this, std::placeholders::_1);
	gRecordsystem->AddEventHandler(new Q3EventHandler(GAME_CLIENT_USERINFO_CHANGED, EXECUTE_TYPE_BEFORE, clientUserInfoChangedHook));
	
	auto clientBeginHook = std::bind(&UserManagementPlugin::OnClientBegin, this, std::placeholders::_1);
	gRecordsystem->AddEventHandler(new Q3EventHandler(GAME_CLIENT_BEGIN, EXECUTE_TYPE_BEFORE, clientBeginHook));
	
	// defrag hook to hide defrag the original userinfostring
	auto gGetUserInfoHook = std::bind(&UserManagementPlugin::OnSyscallGetUserInfo, this, std::placeholders::_1);
	gRecordsystem->GetVmSyscalls()->AddEventHandler(new Q3EventHandler(G_GET_USERINFO, EXECUTE_TYPE_BEFORE, gGetUserInfoHook));

	// grep the userinfostring if defrag change them.
	auto gSetUserInfoHook = std::bind(&UserManagementPlugin::OnSyscallSetUserInfo, this, std::placeholders::_1);
	gRecordsystem->GetVmSyscalls()->AddEventHandler(new Q3EventHandler(G_SET_USERINFO, EXECUTE_TYPE_BEFORE, gSetUserInfoHook));

	// save userid if proxymod is shutingdown...
	auto gameShutdownHook = std::bind(&UserManagementPlugin::OnVmMainShutdown, this, std::placeholders::_1);
	gRecordsystem->AddEventHandler(new Q3EventHandler(GAME_SHUTDOWN, EXECUTE_TYPE_BEFORE, gameShutdownHook));

	gRecordsystem->RegisterCvar((vmCvar_t *)&cvarRsNoNewClients, "rs_no_new_clients", "0", 0, qfalse);
	gRecordsystem->RegisterCvar((vmCvar_t *)&cvarRsWelcome1, "rs_welcome1", "q3df - recordsystem - v0.1 (alpha)", 0, qfalse);
	gRecordsystem->RegisterCvar((vmCvar_t *)&cvarRsWelcome2, "rs_welcome2", "", 0, qfalse);
	gRecordsystem->RegisterCvar((vmCvar_t *)&cvarRsWelcome3, "rs_welcome3", "", 0, qfalse);
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
	cl->SetUniqueId(gRecordsystem->GetSyscalls()->Milliseconds());

	OnClientUserInfoChanged(e); // we can forward because param 1 is our playernum!

	if(isFirstTime) {
		cl->SetState(CLIENT_CONNECTED);
		gRecordsystem->GetSyscalls()->Print(va("client:%i connecting (%s)...\n", cl->GetPlayernum(), cl->GetUserInfo("name")));

		nullRes = new NullResponse();
		clReq = new ClientInfoRequest();
		cl->WriteIdentifier(clReq->mutable_identifier());
		clReq->set_userid(0);
		clReq->set_userinfostring(cl->GetUserInfoString());

		EXECUTE_API_ASYNC(&Q3dfApi_Stub::ClientConnected, clReq, nullRes, NULL);
	} else {
		// restore client
		cl->SetState(CLIENT_ACTIVE);
		cl->SetUserId(gRecordsystem->GetSyscalls()->CvarVariableIntegerValue(va("loginstate%i", playernum)));
		gRecordsystem->GetSyscalls()->Print(va("client:%i restoring (%s)...\n", cl->GetPlayernum(), cl->GetUserInfo("name")));
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
	gRecordsystem->GetSyscalls()->CvarSet(va("loginstate%i", playernum), "0");
}


void UserManagementPlugin::OnClientUserInfoChanged(Q3EventArgs *e) {
	static char tmpBuffer[BIG_INFO_STRING];
	Q3User *cl = NULL;
	LoginRequest *lReq = NULL;
	LoginResponse *lRes = NULL;

	int playernum = e->GetParam(0);

	gRecordsystem->GetSyscalls()->GetUserinfo(playernum, tmpBuffer, sizeof(tmpBuffer));

	cl = gRecordsystem->GetUser(playernum);
	cl->SetUserInfoString(tmpBuffer); // will be copied!

	const char *q3dfkey = cl->GetUserInfo("q3dfkey");
	if((!q3dfkey || strlen(q3dfkey) <= 0) && cl->GetUserId() != 0) {
		cl->SetUserId(0);
		gRecordsystem->GetSyscalls()->SendServerCommand(cl->GetPlayernum(), "print \"^7[^3Q3df::Info^7] you're currently logged out...\n\"");
	}else if(cl->GetUserId() == 0 || strcmp(cl->GetLastQ3dfkey(), q3dfkey)) {
		lReq = new LoginRequest();
		cl->WriteIdentifier(lReq->mutable_identifier());
		lReq->set_q3dfkey(q3dfkey);

		lRes = new LoginResponse();

		EXECUTE_API_ASYNC(&Q3dfApi_Stub::Login, lReq, lRes, [](Message *msg, rpc::Error *error) {
			LoginResponse *res = (LoginResponse *)msg;

			if(!error->IsNil()) {
				gRecordsystem->GetSyscalls()->SendServerCommand(
					res->identifier().playernum(), 
					va("print \"^7[^1Q3df::Error^7] %s\n\"", error->String().c_str())
				);
				gRecordsystem->GetSyscalls()->SendServerCommand(
					res->identifier().playernum(), 
					"print \"^7[^3Q3df::Info^7] you're currently ^1NOT^7 logged in!\n\""
				);
				gRecordsystem->GetUser(res->identifier().playernum())->SetUserId(0);
				return;
			}else{
				gRecordsystem->GetSyscalls()->SendServerCommand(
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
		gRecordsystem->GetSyscalls()->SendServerCommand(playernum, va("chat \"[Q3df] %s\"", cvarRsWelcome1.string));

	if(strlen(cvarRsWelcome2.string) > 0)
		gRecordsystem->GetSyscalls()->SendServerCommand(playernum, va("chat \"[Q3df] %s\"", cvarRsWelcome2.string));

	if(strlen(cvarRsWelcome3.string) > 0)
		gRecordsystem->GetSyscalls()->SendServerCommand(playernum, va("chat \"[Q3df] %s\"", cvarRsWelcome3.string));

	if(cl->GetUserId() > 0)
		gRecordsystem->GetSyscalls()->SendServerCommand(playernum, va("print \"^7[^3Q3df::Info^7] you're currently logged in with userid ^3%i^7.\n\"", cl->GetUserId()));
	else
		gRecordsystem->GetSyscalls()->SendServerCommand(playernum, va("print \"^7[^3Q3df::Info^7] you're ^1NOT^7 logged in!\n\"", cl->GetUserId()));
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
			gRecordsystem->GetSyscalls()->CvarSet(va("loginstate%i", i), va("%i", cl->GetUserId()));
	}
}


// Register plugin!
static PluginProxy<UserManagementPlugin> gUserManagementPlugin;
