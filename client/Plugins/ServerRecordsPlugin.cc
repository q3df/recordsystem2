#include <stdio.h>
#include "Plugin.h"
#include "../Recordsystem.h"
#include "../Logger.h"
#include "../StringTokenizer.h"

class ServerRecordsPlugin : public PluginBase {
private:
	vmCvar_t cvarRsEnabled;

public:
	virtual void Init() {
		gRecordsystem->RegisterCvar((vmCvar_t *)&this->cvarRsEnabled, "rs_enabled", "0", 0, qfalse);

		auto onGamePrintHandler = std::bind(&ServerRecordsPlugin::OnGamePrint, this, std::placeholders::_1);
		gRecordsystem->GetVmSyscalls()->AddEventHandler(new Q3EventHandler(G_PRINT, EXECUTE_TYPE_BEFORE, onGamePrintHandler));
	}

	virtual void Destroy() {
	}

	virtual void OnGamePrint(Q3EventArgs *e) {
		char checksum[1024];
		Q3User *cl = NULL;
		RecordRequest *req = NULL;
		NullResponse *nullRes = NULL;
		StringTokenizer *tokenizer = NULL;

		const char *msg = (const char *)e->GetParamVMA(0);

		if(!strncmp("ClientTimerStop:", msg, 16) && cvarRsEnabled.integer == 1) {
			int check_interferenceoff =  gRecordsystem->GetSyscalls()->CvarVariableIntegerValue("df_mp_interferenceOff");
			int check_msec = gRecordsystem->GetSyscalls()->CvarVariableIntegerValue("pmove_msec");
			int check_fixed = gRecordsystem->GetSyscalls()->CvarVariableIntegerValue("pmove_fixed");
			int check_timescale = gRecordsystem->GetSyscalls()->CvarVariableIntegerValue("timescale");
			int check_speed = gRecordsystem->GetSyscalls()->CvarVariableIntegerValue("g_speed");
			int check_gravity = gRecordsystem->GetSyscalls()->CvarVariableIntegerValue("g_gravity");
			int check_knockback = gRecordsystem->GetSyscalls()->CvarVariableIntegerValue("g_knockback");

			if(check_knockback != 1000 || check_gravity != 800 || check_speed != 320 || check_timescale != 1 || check_fixed != 1 || check_msec != 8 || check_interferenceoff != 3) {
				gRecordsystem->GetSyscalls()->SendServerCommand(-1, "chat \"^7[^1Q3df::Error^7]: --------- Recordsystem ^1DISABLED^7! ----------\"");
				gRecordsystem->GetSyscalls()->SendServerCommand(-1, "chat \"^7[^1Q3df::Error^7]: ---------- CHECK SERVER SETTINGS -------------\"");
				gRecordsystem->GetSyscalls()->SendServerCommand(-1, "chat \"^7[^1Q3df::Error^7]: pmove_msec=8, pmove_fixed=1, timescale=1\"");
				gRecordsystem->GetSyscalls()->SendServerCommand(-1, "chat \"^7[^1Q3df::Error^7]: g_speed=320, g_gravity=800, g_knockback=1000\"");
				gRecordsystem->GetSyscalls()->SendServerCommand(-1, "chat \"^7[^1Q3df::Error^7]: df_mp_interferenceOff=3, sv_cheats=0\"");
			}else{
				req = new RecordRequest();
				nullRes = new NullResponse();
				tokenizer = new StringTokenizer(msg, false);

				cl = gRecordsystem->GetUser(atoi(tokenizer->Argv(1)));
				cl->WriteIdentifier(req->mutable_identifier());

				req->set_name(tokenizer->Argv(4));
				req->set_mapname(tokenizer->Argv(3));
				req->set_mstime(atoi(tokenizer->Argv(2)));
				req->set_df_gametype(atoi(tokenizer->Argv(5)));
				req->set_df_promode(atoi(tokenizer->Argv(6)));
				req->set_df_mode(atoi(tokenizer->Argv(7)));
				req->set_interference(atoi(tokenizer->Argv(8)));
				req->set_obs_enabled(atoi(tokenizer->Argv(9)));
				req->set_defrag_version(atoi(tokenizer->Argv(10)));

				gRecordsystem->GetSyscalls()->CvarVariableStringBuffer("sv_mapChecksum", checksum, sizeof(checksum));
				req->set_map_checksum(checksum);

				EXECUTE_API_ASYNC(&Q3dfApi_Stub::SaveRecord, req, nullRes, NULL);
				delete tokenizer;
			}
		}else if(!strncmp("ClientTimerStop:", msg, 16) && cvarRsEnabled.integer == 0) {
			gRecordsystem->GetSyscalls()->SendServerCommand(-1, "chat \"^7[^3Q3df::Info^7]: Recordsystem ^1DISABLED^7! (rs_enabled = 0)\"");
		}
	}
};

// Register plugin!
static PluginProxy<ServerRecordsPlugin> gServerRecordsPlugin;
