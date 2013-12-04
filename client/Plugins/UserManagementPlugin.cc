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

	void OnClientConnected(Q3Hook *hook);
	void OnClientDisconnected(Q3Hook *hook);
	void OnClientUserInfoChanged(Q3Hook *hook);
	void OnClientBegin(Q3Hook *hook);

public:
	virtual void Init() {
		auto clientConnectedHook = std::bind(&UserManagementPlugin::OnClientConnected, this, std::placeholders::_1);
		auto clientDisconnectedHook = std::bind(&UserManagementPlugin::OnClientDisconnected, this, std::placeholders::_1);
		auto clientUserInfoChangedHook = std::bind(&UserManagementPlugin::OnClientUserInfoChanged, this, std::placeholders::_1);
		auto clientBeginHook = std::bind(&UserManagementPlugin::OnClientBegin, this, std::placeholders::_1);

		gRecordsystem->AddHook(new Q3Hook(GAME_CLIENT_CONNECT, EXECUTE_TYPE_BEFORE, clientConnectedHook));
		gRecordsystem->AddHook(new Q3Hook(GAME_CLIENT_DISCONNECT, EXECUTE_TYPE_BEFORE, clientDisconnectedHook));
		gRecordsystem->AddHook(new Q3Hook(GAME_CLIENT_USERINFO_CHANGED, EXECUTE_TYPE_BEFORE, clientUserInfoChangedHook));
		gRecordsystem->AddHook(new Q3Hook(GAME_CLIENT_BEGIN, EXECUTE_TYPE_BEFORE, clientBeginHook));

		// defrag hook to hide defrag the original userinfostring
		gRecordsystem->GetVmSyscalls()->addHook(new Q3Hook(G_GET_USERINFO, EXECUTE_TYPE_BEFORE, [](Q3Hook *hook) {
			int playernum = hook->getParam(0);
			int bufferSize = hook->getParam(2);
			char *buffer = (char *)hook->getParamPtr(1);

			Q3User *cl = gRecordsystem->GetUser(playernum);

			strncpy(buffer, cl->GetUserInfoString(), bufferSize);
			hook->setHandled(true);
		}));

		// grep the userinfostring if defrag change them.
		gRecordsystem->GetVmSyscalls()->addHook(new Q3Hook(G_SET_USERINFO, EXECUTE_TYPE_BEFORE, [](Q3Hook *hook) {
			static char tmpBuffer[BIG_INFO_STRING];
			int playernum = hook->getParam(0);
			const char *buffer = (const char *)hook->getParamPtr(1);

			strncpy(tmpBuffer, buffer, sizeof(tmpBuffer));
			gRecordsystem->GetUser(playernum)->SetUserInfoString(tmpBuffer);
		}));

		gRecordsystem->RegisterCvar((vmCvar_t *)&cvarRsNoNewClients, "rs_no_new_clients", "0", 0, qfalse);
		gRecordsystem->RegisterCvar((vmCvar_t *)&cvarRsWelcome1, "rs_welcome1", "q3df - recordsystem - v0.1 (alpha)", 0, qfalse);
		gRecordsystem->RegisterCvar((vmCvar_t *)&cvarRsWelcome2, "rs_welcome2", "", 0, qfalse);
		gRecordsystem->RegisterCvar((vmCvar_t *)&cvarRsWelcome3, "rs_welcome3", "", 0, qfalse);
	}

	virtual void Destroy() {
	}


	/*************************************************************
	 * Eventhandling!
	 */
};


void UserManagementPlugin::OnClientConnected(Q3Hook *hook) {
	Q3User *cl = NULL;
	int playernum = hook->getParam(0);
	bool isFirstTime = hook->getParam(1) == 1 ? true : false;
	bool isBot = hook->getParam(2) == 1 ? true : false;

	if(cvarRsNoNewClients.integer == 1) {
		hook->setReturnVMA("Sorry, server in latched latched for shutdown.");
		return;
	}

	cl = gRecordsystem->GetUser(playernum);
	cl->Reset();

	OnClientUserInfoChanged(hook); // we can forward because param 1 is our playernum!

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

void UserManagementPlugin::OnClientDisconnected(Q3Hook *hook) {
	int playernum = hook->getParam(0);
	Q3User *cl = gRecordsystem->GetUser(playernum);
	cl->Reset();
	gRecordsystem->GetSyscalls()->CvarSet(va("loginstate%i", playernum), "0");
}

void UserManagementPlugin::OnClientUserInfoChanged(Q3Hook *hook) {
	static char tmpBuffer[BIG_INFO_STRING];
	Q3User *cl = NULL;

	int playernum = hook->getParam(0);

	gRecordsystem->GetSyscalls()->GetUserinfo(playernum, tmpBuffer, sizeof(tmpBuffer));

	cl = gRecordsystem->GetUser(playernum);
	cl->SetUserInfoString(tmpBuffer); // will be copied!
	cl->SetUserInfo("name", va("nightmare_test_by_code_%i", playernum));

	gRecordsystem->GetSyscalls()->Print(va("client:%i userinfo changed...\n", playernum));
}

void UserManagementPlugin::OnClientBegin(Q3Hook *hook) {
	int playernum = hook->getParam(0);
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

// Register plugin!
static PluginProxy<UserManagementPlugin> gUserManagementPlugin;
