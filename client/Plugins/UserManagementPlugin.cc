#include <stdio.h>
#include "Plugin.h"
#include "../Recordsystem.h"
#include "../Logger.h"
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
	int playernum = e->GetParam(0);
	bool isFirstTime = e->GetParam(1) == 1 ? true : false;
	//bool isBot = e->GetParam(2) == 1 ? true : false;

	if(cvarRsNoNewClients.integer == 1) {
		e->SetReturn((void *)"Sorry, server in latched for shutdown.");
		return;
	}

	cl = gRecordsystem->GetUser(playernum);
	cl->Reset();

	OnClientUserInfoChanged(e); // we can forward because param 1 is our playernum!

	if(isFirstTime) {
		cl->SetState(CLIENT_CONNECTED);
		gRecordsystem->GetSyscalls()->Print(va("client:%i connecting (%s)...\n", cl->GetPlayernum(), cl->GetUserInfo("name")));
	} else {
		// restore client
		cl->SetState(CLIENT_ACTIVE);
		cl->SetUserId(gRecordsystem->GetSyscalls()->CvarVariableIntegerValue(va("loginstate%i", playernum)));
		gRecordsystem->GetSyscalls()->Print(va("client:%i restoring (%s)...\n", cl->GetPlayernum(), cl->GetUserInfo("name")));
	}
}


void UserManagementPlugin::OnClientDisconnected(Q3EventArgs *e) {
	int playernum = e->GetParam(0);
	Q3User *cl = gRecordsystem->GetUser(playernum);
	cl->Reset();
	gRecordsystem->GetSyscalls()->CvarSet(va("loginstate%i", playernum), "0");
}


void UserManagementPlugin::OnClientUserInfoChanged(Q3EventArgs *e) {
	static char tmpBuffer[BIG_INFO_STRING];
	Q3User *cl = NULL;

	int playernum = e->GetParam(0);

	gRecordsystem->GetSyscalls()->GetUserinfo(playernum, tmpBuffer, sizeof(tmpBuffer));

	cl = gRecordsystem->GetUser(playernum);
	cl->SetUserInfoString(tmpBuffer); // will be copied!
	cl->SetUserInfo("name", va("nightmare_test_by_code_%i", playernum));

	gRecordsystem->GetSyscalls()->Print(va("client:%i userinfo changed...\n", playernum));
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

	gRecordsystem->GetSyscalls()->Print(va("client:%i begin...\n", playernum));
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
