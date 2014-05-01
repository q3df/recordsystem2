#ifndef CLIENT_RECORDSYSTEM_H_
#define CLIENT_RECORDSYSTEM_H_

#include "Quake3.h"
#include "Q3SysCall.h"
#include "Q3Vm.h"
#include "ApiAsyncExecuter.h"
#include "Q3Event.h"
#include "VmCvarItem.h"

#include <utf8.h>
#include <functional>
#include <list>
#include <vector>

#include <q3df_api.pb.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

#include "PluginStore.h"
#include "PluginBase.h"
#include "PluginProxy.h"
#include "Q3User.h"
#include "SqliteDatabase.h"

using namespace google::protobuf;
using namespace service;

extern vmCvar_t rs_api_server;
extern vmCvar_t rs_api_port;
extern vmCvar_t rs_api_key;

extern "C" {
	void fix_utf8_string(std::string& str);
	const char *va( const char *format, ... );
}

class Recordsystem {
friend class Q3User;
public:
	Recordsystem(syscall_t syscall);
	~Recordsystem();

	int GetUserCount();
	Q3User *GetUser(int playernum);
	void RegisterCvar(vmCvar_t *cvarPtr, const char name[MAX_CVAR_VALUE_STRING], const char defaultValue[MAX_CVAR_VALUE_STRING], int flags, qboolean track);
	ApiAsyncExecuter *GetAsyncExecuter();
	Q3dfApi_Stub *GetQ3dfApi();
	Q3SysCall *GetSyscalls();
	Q3SysCall *GetVmSyscalls();
	void AddEventHandler(Q3EventHandler *eventItem);
	void RemoveEventHandler(Q3EventHandler *eventItem);
	SqliteDatabase *DB();
	int VmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);

private:
	playerState_t *GetPlayerstate(int playernum);
	bool GameInit(int levelTime, int randomSeed, int restart);
	void SetGameData(int clientSize, playerState_t *clients, gentity_t *gentities, int entitiesSize, int numEntities);
	void UpdateQuake3Cvars();
	ApiAsyncExecuter *asyncExec_;
	Q3SysCall *vm_syscall_;
	Q3SysCall *syscall_;
	Q3Vm *vm_;
	rpc::Client *apiClient_;
	Q3dfApi_Stub *Q3dfApi_;


	int gameClientSize_;
	playerState_t *gameClients_;
	gentity_t *gentities_;
	int	gentitySize_;
	int	numEntities_;

	EventHandlers eventList_;
	list<VmCvarItem *> cvarList_;
	list<PluginBase *> pluginList_;
	vector<Q3User *> userList_;
	SqliteDatabase *db_;
};

extern Recordsystem *gRecordsystem;

#endif // CLIENT_RECORDSYSTEM_H_

