#include "Q3SysCall.h"
#include "Recordsystem.h"

extern "C" {
	void fix_utf8_string(std::string& str);
	const char *va( const char *format, ... );
}

int PASSFLOAT(float x) {
	float	floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

Q3SysCall::Q3SysCall(syscall_t syscall) : 
	syscall_(syscall) {
}

Q3SysCall::~Q3SysCall() {
	Q3EventHandler *eventItem = NULL;

	while(!eventList_.empty()) {
		eventItem = eventList_.back();
		eventList_.pop_back();
		delete eventItem;
	}

	eventList_.clear();
}

void Q3SysCall::AddEventHandler(Q3EventHandler *eventItem) {
	eventList_.push_back(eventItem);
}

void Q3SysCall::RemoveEventHandler(Q3EventHandler *eventItem) {
	gRecordsystem->GetSyscalls()->PrintError("Q3SysCall::RemoveEvent not implemented\n");
}


void Q3SysCall::Printf(const char *fmt) {
	Q3EventHandler *eventItem = NULL;
	EXECUTE_EVENT_VOID_ARG1(G_PRINT, EXECUTE_TYPE_BEFORE, (void *)fmt)

	if(!eventItem || (eventItem && !eventItem->GetHandled()))
		syscall_(G_PRINT, fmt);

	EXECUTE_EVENT_VOID_ARG1(G_PRINT, EXECUTE_TYPE_AFTER, (void *)fmt)
}

void Q3SysCall::Print(const char *fmt) {
	syscall_(G_PRINT, va("[Q3df] %s", fmt));
}

void Q3SysCall::PrintWarning(const char *fmt) {
	syscall_(G_PRINT, va("[Q3df:WARNING] %s", fmt));
}

void Q3SysCall::PrintError(const char *fmt) {
	syscall_(G_PRINT, va("[Q3df:ERROR] %s", fmt));
}


void Q3SysCall::Error(const char *fmt) {
	syscall_(G_ERROR, fmt);
}

int Q3SysCall::Milliseconds(void) {
	return syscall_(G_MILLISECONDS);
}

int Q3SysCall::Argc(void) {
	return syscall_(G_ARGC);
}

void Q3SysCall::Argv(int n, char *buffer, int bufferLength) {
	syscall_(G_ARGV, n, buffer, bufferLength);
}

int Q3SysCall::FSFOpenFile(const char *qpath, fileHandle_t *f, fsMode_t mode) {
	return syscall_(G_FS_FOPEN_FILE, qpath, f, mode);
}

void Q3SysCall::FSRead(void *buffer, int len, fileHandle_t f) {
	syscall_(G_FS_READ, buffer, len, f);
}

void Q3SysCall::FSWrite(const void *buffer, int len, fileHandle_t f) {
	syscall_(G_FS_WRITE, buffer, len, f);
}

void Q3SysCall::FSFCloseFile(fileHandle_t f) {
	syscall_(G_FS_FCLOSE_FILE, f);
}

int Q3SysCall::FSGetFileList( const char *path, const char *extension, char *listbuf, int bufsize) {
	return syscall_(G_FS_GETFILELIST, path, extension, listbuf, bufsize);
}

int Q3SysCall::FSSeek(fileHandle_t f, long offset, int origin) {
	return syscall_(G_FS_SEEK, f, offset, origin);
}

void Q3SysCall::SendConsoleCommand(int exec_when, const char *text) {
	syscall_(G_SEND_CONSOLE_COMMAND, exec_when, text);
}

void Q3SysCall::CvarRegister(vmCvar_t *cvar, const char *var_name, const char *value, int flags) {
	syscall_(G_CVAR_REGISTER, cvar, var_name, value, flags);
}

void Q3SysCall::CvarUpdate(vmCvar_t *cvar) {
	syscall_(G_CVAR_UPDATE, cvar);
}

void Q3SysCall::CvarSet(const char *var_name, const char *value) {
	syscall_(G_CVAR_SET, var_name, value);
}

int Q3SysCall::CvarVariableIntegerValue(const char *var_name) {
	return syscall_(G_CVAR_VARIABLE_INTEGER_VALUE, var_name);
}

void Q3SysCall::CvarVariableStringBuffer(const char *var_name, char *buffer, int bufsize) {
	syscall_(G_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize);
}

void Q3SysCall::LocateGameData(gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGClient) {
	Q3EventHandler *eventItem;
	EXECUTE_EVENT_VOID_ARG5(G_LOCATE_GAME_DATA, EXECUTE_TYPE_BEFORE, (void *)gEnts, numGEntities, sizeofGEntity_t, (void *)clients, sizeofGClient)

	if(!eventItem || (eventItem && !eventItem->GetHandled()))
		syscall_(G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient);

	EXECUTE_EVENT_VOID_ARG5(G_LOCATE_GAME_DATA, EXECUTE_TYPE_AFTER, (void *)gEnts, numGEntities, sizeofGEntity_t, (void *)clients, sizeofGClient)
}

void Q3SysCall::DropClient(int clientNum, const char *reason) {
	syscall_(G_DROP_CLIENT, clientNum, reason);
}

void Q3SysCall::SendServerCommand(int clientNum, const char *text) {
	syscall_(G_SEND_SERVER_COMMAND, clientNum, text);
}

void Q3SysCall::SetConfigstring(int num, const char *string) {
	syscall_(G_SET_CONFIGSTRING, num, string);
}

void Q3SysCall::GetConfigstring(int num, char *buffer, int bufferSize) {
	syscall_(G_GET_CONFIGSTRING, num, buffer, bufferSize);
}

void Q3SysCall::GetUserinfo(int num, char *buffer, int bufferSize) {
	Q3EventHandler *eventItem = NULL;
	EXECUTE_EVENT_VOID_ARG3(G_GET_USERINFO, EXECUTE_TYPE_BEFORE, num, (void *)buffer, bufferSize)

	if(!eventItem || (eventItem && !eventItem->GetHandled()))
		syscall_(G_GET_USERINFO, num, buffer, bufferSize);

	EXECUTE_EVENT_VOID_ARG3(G_GET_USERINFO, EXECUTE_TYPE_AFTER, num, (void *)buffer, bufferSize)
}

void Q3SysCall::SetUserinfo(int num, const char *buffer) {
	Q3EventHandler *eventItem;
	EXECUTE_EVENT_VOID_ARG2(G_SET_USERINFO, EXECUTE_TYPE_BEFORE, num, (void *)buffer)

	if(!eventItem || (eventItem && !eventItem->GetHandled()))
		syscall_(G_SET_USERINFO, num, buffer);

	EXECUTE_EVENT_VOID_ARG2(G_SET_USERINFO, EXECUTE_TYPE_AFTER, num, (void *)buffer)
}

void Q3SysCall::GetServerinfo(char *buffer, int bufferSize) {
	syscall_(G_GET_SERVERINFO, buffer, bufferSize);
}

void Q3SysCall::SetBrushModel(gentity_t *ent, const char *name) {
	syscall_(G_SET_BRUSH_MODEL, ent, name);
}

void Q3SysCall::Trace(trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask) {
	syscall_(G_TRACE, results, start, mins, maxs, end, passEntityNum, contentmask);
}

void Q3SysCall::TraceCapsule(trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask) {
	syscall_(G_TRACECAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask);
}

int Q3SysCall::PointContents(const vec3_t point, int passEntityNum) {
	return syscall_(G_POINT_CONTENTS, point, passEntityNum);
}

qboolean Q3SysCall::InPVS(const vec3_t p1, const vec3_t p2) {
	return (qboolean)syscall_(G_IN_PVS, p1, p2);
}

qboolean Q3SysCall::InPVSIgnorePortals(const vec3_t p1, const vec3_t p2) {
	return (qboolean)syscall_(G_IN_PVS_IGNORE_PORTALS, p1, p2);
}

void Q3SysCall::AdjustAreaPortalState(gentity_t *ent, qboolean open) {
	syscall_(G_ADJUST_AREA_PORTAL_STATE, ent, open);
}

qboolean Q3SysCall::AreasConnected(int area1, int area2) {
	return (qboolean)syscall_(G_AREAS_CONNECTED, area1, area2);
}

void Q3SysCall::LinkEntity(gentity_t *ent) {
	syscall_(G_LINKENTITY, ent);
}

void Q3SysCall::UnlinkEntity(gentity_t *ent) {
	syscall_(G_UNLINKENTITY, ent);
}

int Q3SysCall::EntitiesInBox(const vec3_t mins, const vec3_t maxs, gentity_t **list, int maxcount) {
	return syscall_(G_ENTITIES_IN_BOX, mins, maxs, list, maxcount);
}

qboolean Q3SysCall::EntityContact(const vec3_t mins, const vec3_t maxs, const gentity_t *ent) {
	return (qboolean)syscall_(G_ENTITY_CONTACT, mins, maxs, ent);
}

qboolean Q3SysCall::EntityContactCapsule(const vec3_t mins, const vec3_t maxs, const gentity_t *ent) {
	return (qboolean)syscall_(G_ENTITY_CONTACTCAPSULE, mins, maxs, ent);
}

int Q3SysCall::BotAllocateClient(void) {
	return syscall_(G_BOT_ALLOCATE_CLIENT);
}

void Q3SysCall::BotFreeClient(int clientNum) {
	syscall_(G_BOT_FREE_CLIENT, clientNum);
}

void Q3SysCall::GetUsercmd(int clientNum, usercmd_t *cmd) {
	syscall_(G_GET_USERCMD, clientNum, cmd);
}

qboolean Q3SysCall::GetEntityToken(char *buffer, int bufferSize) {
	return (qboolean)syscall_(G_GET_ENTITY_TOKEN, buffer, bufferSize);
}

int Q3SysCall::DebugPolygonCreate(int color, int numPoints, vec3_t *points) {
	return syscall_(G_DEBUG_POLYGON_CREATE, color, numPoints, points);
}

void Q3SysCall::DebugPolygonDelete(int id) {
	syscall_(G_DEBUG_POLYGON_DELETE, id);
}

int Q3SysCall::RealTime(qtime_t *qtime) {
	return syscall_(G_REAL_TIME, qtime);
}

void Q3SysCall::SnapVector(float *v) {
	syscall_(G_SNAPVECTOR, v);
}

// BotLib traps start here
int Q3SysCall::BotLibSetup(void) {
	return syscall_(BOTLIB_SETUP);
}

int Q3SysCall::BotLibShutdown(void) {
	return syscall_(BOTLIB_SHUTDOWN);
}

int Q3SysCall::BotLibVarSet(char *var_name, char *value) {
	return syscall_(BOTLIB_LIBVAR_SET, var_name, value);
}

int Q3SysCall::BotLibVarGet(char *var_name, char *value, int size) {
	return syscall_(BOTLIB_LIBVAR_GET, var_name, value, size);
}

int Q3SysCall::BotLibDefine(char *string) {
	return syscall_(BOTLIB_PC_ADD_GLOBAL_DEFINE, string);
}

int Q3SysCall::BotLibStartFrame(float time) {
	return syscall_(BOTLIB_START_FRAME, PASSFLOAT(time ));
}

int Q3SysCall::BotLibLoadMap(const char *mapname) {
	return syscall_(BOTLIB_LOAD_MAP, mapname);
}

int Q3SysCall::BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue) {
	return syscall_(BOTLIB_UPDATENTITY, ent, bue);
}

int Q3SysCall::BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3) {
	return syscall_(BOTLIB_TEST, parm0, parm1, parm2, parm3);
}

int Q3SysCall::BotGetSnapshotEntity(int clientNum, int sequence) {
	return syscall_(BOTLIB_GET_SNAPSHOT_ENTITY, clientNum, sequence);
}

int Q3SysCall::BotGetServerCommand(int clientNum, char *message, int size) {
	return syscall_(BOTLIB_GET_CONSOLE_MESSAGE, clientNum, message, size);
}

void Q3SysCall::BotUserCommand(int clientNum, usercmd_t *ucmd) {
	syscall_(BOTLIB_USER_COMMAND, clientNum, ucmd);
}

void Q3SysCall::AASEntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info) {
	syscall_(BOTLIB_AAS_ENTITY_INFO, entnum, info);
}

int Q3SysCall::AASInitialized(void) {
	return syscall_(BOTLIB_AAS_INITIALIZED);
}

void Q3SysCall::AASPresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs) {
	syscall_(BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX, presencetype, mins, maxs);
}

float Q3SysCall::AASTime(void) {
	int temp;
	temp = syscall_(BOTLIB_AAS_TIME);
	return (*(float*)&temp);
}

int Q3SysCall::AASPointAreaNum(vec3_t point) {
	return syscall_(BOTLIB_AAS_POINT_AREA_NUM, point);
}

int Q3SysCall::AASPointReachabilityAreaIndex(vec3_t point) {
	return syscall_(BOTLIB_AAS_POINT_REACHABILITY_AREA_INDEX, point);
}

int Q3SysCall::AASTraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas) {
	return syscall_(BOTLIB_AAS_TRACE_AREAS, start, end, areas, points, maxareas);
}

int Q3SysCall::AASBBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas) {
	return syscall_(BOTLIB_AAS_BBOX_AREAS, absmins, absmaxs, areas, maxareas);
}

int Q3SysCall::AASAreaInfo(int areanum, void /* struct aas_areainfo_s */ *info) {
	return syscall_(BOTLIB_AAS_AREA_INFO, areanum, info);
}

int Q3SysCall::AASPointContents(vec3_t point) {
	return syscall_(BOTLIB_AAS_POINT_CONTENTS, point);
}

int Q3SysCall::AASNextBSPEntity(int ent) {
	return syscall_(BOTLIB_AAS_NEXT_BSP_ENTITY, ent);
}

int Q3SysCall::AASValueForBSPEpairKey(int ent, char *key, char *value, int size) {
	return syscall_(BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY, ent, key, value, size);
}

int Q3SysCall::AASVectorForBSPEpairKey(int ent, char *key, vec3_t v) {
	return syscall_(BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY, ent, key, v);
}

int Q3SysCall::AASFloatForBSPEpairKey(int ent, char *key, float *value) {
	return syscall_(BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY, ent, key, value);
}

int Q3SysCall::AASIntForBSPEpairKey(int ent, char *key, int *value) {
	return syscall_(BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY, ent, key, value);
}

int Q3SysCall::AASAreaReachability(int areanum) {
	return syscall_(BOTLIB_AAS_AREA_REACHABILITY, areanum);
}

int Q3SysCall::AASAreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags) {
	return syscall_(BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA, areanum, origin, goalareanum, travelflags);
}

int Q3SysCall::AASEnableRoutingArea(int areanum, int enable) {
	return syscall_(BOTLIB_AAS_ENABLE_ROUTING_AREA, areanum, enable);
}

int Q3SysCall::AASPredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin, int goalareanum, int travelflags, int maxareas, int maxtime, int stopevent, int stopcontents, int stoptfl, int stopareanum) {
	return syscall_(BOTLIB_AAS_PREDICT_ROUTE, route, areanum, origin, goalareanum, travelflags, maxareas, maxtime, stopevent, stopcontents, stoptfl, stopareanum);
}

int Q3SysCall::AASAlternativeRouteGoals(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags, void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,	int type) {
	return syscall_(BOTLIB_AAS_ALTERNATIVE_ROUTE_GOAL, start, startareanum, goal, goalareanum, travelflags, altroutegoals, maxaltroutegoals, type);
}

int Q3SysCall::AASSwimming(vec3_t origin) {
	return syscall_(BOTLIB_AAS_SWIMMING, origin);
}

int Q3SysCall::AASPredictClientMovement(void /* struct aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize) {
	return syscall_(BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT, move, entnum, origin, presencetype, onground, velocity, cmdmove, cmdframes, maxframes, PASSFLOAT(frametime), stopevent, stopareanum, visualize);
}

void Q3SysCall::EASay(int client, char *str) {
	syscall_(BOTLIB_EA_SAY, client, str);
}

void Q3SysCall::EASayTeam(int client, char *str) {
	syscall_(BOTLIB_EA_SAY_TEAM, client, str);
}

void Q3SysCall::EACommand(int client, char *command) {
	syscall_(BOTLIB_EA_COMMAND, client, command);
}

void Q3SysCall::EAAction(int client, int action) {
	syscall_(BOTLIB_EA_ACTION, client, action);
}

void Q3SysCall::EAGesture(int client) {
	syscall_(BOTLIB_EA_GESTURE, client);
}

void Q3SysCall::EATalk(int client) {
	syscall_(BOTLIB_EA_TALK, client);
}

void Q3SysCall::EAAttack(int client) {
	syscall_(BOTLIB_EA_ATTACK, client);
}

void Q3SysCall::EAUse(int client) {
	syscall_(BOTLIB_EA_USE, client);
}

void Q3SysCall::EARespawn(int client) {
	syscall_(BOTLIB_EA_RESPAWN, client);
}

void Q3SysCall::EACrouch(int client) {
	syscall_(BOTLIB_EA_CROUCH, client);
}

void Q3SysCall::EAMoveUp(int client) {
	syscall_(BOTLIB_EA_MOVE_UP, client);
}

void Q3SysCall::EAMoveDown(int client) {
	syscall_(BOTLIB_EA_MOVE_DOWN, client);
}

void Q3SysCall::EAMoveForward(int client) {
	syscall_(BOTLIB_EA_MOVE_FORWARD, client);
}

void Q3SysCall::EAMoveBack(int client) {
	syscall_(BOTLIB_EA_MOVE_BACK, client);
}

void Q3SysCall::EAMoveLeft(int client) {
	syscall_(BOTLIB_EA_MOVE_LEFT, client);
}

void Q3SysCall::EAMoveRight(int client) {
	syscall_(BOTLIB_EA_MOVE_RIGHT, client);
}

void Q3SysCall::EASelectWeapon(int client, int weapon) {
	syscall_(BOTLIB_EA_SELECT_WEAPON, client, weapon);
}

void Q3SysCall::EAJump(int client) {
	syscall_(BOTLIB_EA_JUMP, client);
}

void Q3SysCall::EADelayedJump(int client) {
	syscall_(BOTLIB_EA_DELAYED_JUMP, client);
}

void Q3SysCall::EAMove(int client, vec3_t dir, float speed) {
	syscall_(BOTLIB_EA_MOVE, client, dir, PASSFLOAT(speed));
}

void Q3SysCall::EAView(int client, vec3_t viewangles) {
	syscall_(BOTLIB_EA_VIEW, client, viewangles);
}

void Q3SysCall::EAEndRegular(int client, float thinktime) {
	syscall_(BOTLIB_EA_END_REGULAR, client, PASSFLOAT(thinktime));
}

void Q3SysCall::EAGetInput(int client, float thinktime, void /* struct bot_input_s */ *input) {
	syscall_(BOTLIB_EA_GET_INPUT, client, PASSFLOAT(thinktime), input);
}

void Q3SysCall::EAResetInput(int client) {
	syscall_(BOTLIB_EA_RESET_INPUT, client);
}

int Q3SysCall::BotLoadCharacter(char *charfile, float skill) {
	return syscall_(BOTLIB_AI_LOAD_CHARACTER, charfile, PASSFLOAT(skill));
}

void Q3SysCall::BotFreeCharacter(int character) {
	syscall_(BOTLIB_AI_FREE_CHARACTER, character);
}

float Q3SysCall::CharacteristicFloat(int character, int index) {
	int temp;
	temp = syscall_(BOTLIB_AI_CHARACTERISTIC_FLOAT, character, index);
	return (*(float*)&temp);
}

float Q3SysCall::CharacteristicBFloat(int character, int index, float min, float max) {
	int temp;
	temp = syscall_(BOTLIB_AI_CHARACTERISTIC_BFLOAT, character, index, PASSFLOAT(min), PASSFLOAT(max));
	return (*(float*)&temp);
}

int Q3SysCall::CharacteristicInteger(int character, int index) {
	return syscall_(BOTLIB_AI_CHARACTERISTIC_INTEGER, character, index);
}

int Q3SysCall::CharacteristicBInteger(int character, int index, int min, int max) {
	return syscall_(BOTLIB_AI_CHARACTERISTIC_BINTEGER, character, index, min, max);
}

void Q3SysCall::CharacteristicString(int character, int index, char *buf, int size) {
	syscall_(BOTLIB_AI_CHARACTERISTIC_STRING, character, index, buf, size);
}

int Q3SysCall::BotAllocChatState(void) {
	return syscall_(BOTLIB_AI_ALLOC_CHAT_STATE);
}

void Q3SysCall::BotFreeChatState(int handle) {
	syscall_(BOTLIB_AI_FREE_CHAT_STATE, handle);
}

void Q3SysCall::BotQueueConsoleMessage(int chatstate, int type, char *message) {
	syscall_(BOTLIB_AI_QUEUE_CONSOLE_MESSAGE, chatstate, type, message);
}

void Q3SysCall::BotRemoveConsoleMessage(int chatstate, int handle) {
	syscall_(BOTLIB_AI_REMOVE_CONSOLE_MESSAGE, chatstate, handle);
}

int Q3SysCall::BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm) {
	return syscall_(BOTLIB_AI_NEXT_CONSOLE_MESSAGE, chatstate, cm);
}

int Q3SysCall::BotNumConsoleMessages(int chatstate) {
	return syscall_(BOTLIB_AI_NUM_CONSOLE_MESSAGE, chatstate);
}

void Q3SysCall::BotInitialChat(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7) {
	syscall_(BOTLIB_AI_INITIAL_CHAT, chatstate, type, mcontext, var0, var1, var2, var3, var4, var5, var6, var7);
}

int Q3SysCall::BotNumInitialChats(int chatstate, char *type) {
	return syscall_(BOTLIB_AI_NUM_INITIAL_CHATS, chatstate, type);
}

int Q3SysCall::BotReplyChat(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7) {
	return syscall_(BOTLIB_AI_REPLY_CHAT, chatstate, message, mcontext, vcontext, var0, var1, var2, var3, var4, var5, var6, var7);
}

int Q3SysCall::BotChatLength(int chatstate) {
	return syscall_(BOTLIB_AI_CHAT_LENGTH, chatstate);
}

void Q3SysCall::BotEnterChat(int chatstate, int client, int sendto) {
	syscall_(BOTLIB_AI_ENTER_CHAT, chatstate, client, sendto);
}

void Q3SysCall::BotGetChatMessage(int chatstate, char *buf, int size) {
	syscall_(BOTLIB_AI_GET_CHAT_MESSAGE, chatstate, buf, size);
}

int Q3SysCall::StringContains(char *str1, char *str2, int casesensitive) {
	return syscall_(BOTLIB_AI_STRING_CONTAINS, str1, str2, casesensitive);
}

int Q3SysCall::BotFindMatch(char *str, void /* struct bot_match_s */ *match, unsigned long int context) {
	return syscall_(BOTLIB_AI_FIND_MATCH, str, match, context);
}

void Q3SysCall::BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size) {
	syscall_(BOTLIB_AI_MATCH_VARIABLE, match, variable, buf, size);
}

void Q3SysCall::UnifyWhiteSpaces(char *string) {
	syscall_(BOTLIB_AI_UNIFY_WHITE_SPACES, string);
}

void Q3SysCall::BotReplaceSynonyms(char *string, unsigned long int context) {
	syscall_(BOTLIB_AI_REPLACE_SYNONYMS, string, context);
}

int Q3SysCall::BotLoadChatFile(int chatstate, char *chatfile, char *chatname) {
	return syscall_(BOTLIB_AI_LOAD_CHAT_FILE, chatstate, chatfile, chatname);
}

void Q3SysCall::BotSetChatGender(int chatstate, int gender) {
	syscall_(BOTLIB_AI_SET_CHAT_GENDER, chatstate, gender);
}

void Q3SysCall::BotSetChatName(int chatstate, char *name, int client) {
	syscall_(BOTLIB_AI_SET_CHAT_NAME, chatstate, name, client);
}

void Q3SysCall::BotResetGoalState(int goalstate) {
	syscall_(BOTLIB_AI_RESET_GOAL_STATE, goalstate);
}

void Q3SysCall::BotResetAvoidGoals(int goalstate) {
	syscall_(BOTLIB_AI_RESET_AVOID_GOALS, goalstate);
}

void Q3SysCall::BotRemoveFromAvoidGoals(int goalstate, int number) {
	syscall_(BOTLIB_AI_REMOVE_FROM_AVOID_GOALS, goalstate, number);
}

void Q3SysCall::BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	syscall_(BOTLIB_AI_PUSH_GOAL, goalstate, goal);
}

void Q3SysCall::BotPopGoal(int goalstate) {
	syscall_(BOTLIB_AI_POP_GOAL, goalstate);
}

void Q3SysCall::BotEmptyGoalStack(int goalstate) {
	syscall_(BOTLIB_AI_EMPTY_GOAL_STACK, goalstate);
}

void Q3SysCall::BotDumpAvoidGoals(int goalstate) {
	syscall_(BOTLIB_AI_DUMP_AVOID_GOALS, goalstate);
}

void Q3SysCall::BotDumpGoalStack(int goalstate) {
	syscall_(BOTLIB_AI_DUMP_GOAL_STACK, goalstate);
}

void Q3SysCall::BotGoalName(int number, char *name, int size) {
	syscall_(BOTLIB_AI_GOAL_NAME, number, name, size);
}

int Q3SysCall::BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	return syscall_(BOTLIB_AI_GET_TOP_GOAL, goalstate, goal);
}

int Q3SysCall::BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	return syscall_(BOTLIB_AI_GET_SECOND_GOAL, goalstate, goal);
}

int Q3SysCall::BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags) {
	return syscall_(BOTLIB_AI_CHOOSE_LTG_ITEM, goalstate, origin, inventory, travelflags);
}

int Q3SysCall::BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime) {
	return syscall_(BOTLIB_AI_CHOOSE_NBG_ITEM, goalstate, origin, inventory, travelflags, ltg, PASSFLOAT(maxtime));
}

int Q3SysCall::BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal) {
	return syscall_(BOTLIB_AI_TOUCHING_GOAL, origin, goal);
}

int Q3SysCall::BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal) {
	return syscall_(BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE, viewer, eye, viewangles, goal);
}

int Q3SysCall::BotGetLevelItemGoal(int index, char *classname, void /* struct bot_goal_s */ *goal) {
	return syscall_(BOTLIB_AI_GET_LEVEL_ITEM_GOAL, index, classname, goal);
}

int Q3SysCall::BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal) {
	return syscall_(BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL, num, goal);
}

int Q3SysCall::BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal) {
	return syscall_(BOTLIB_AI_GET_MAP_LOCATION_GOAL, name, goal);
}

float Q3SysCall::BotAvoidGoalTime(int goalstate, int number) {
	int temp;
	temp = syscall_(BOTLIB_AI_AVOID_GOAL_TIME, goalstate, number);
	return (*(float*)&temp);
}

void Q3SysCall::BotSetAvoidGoalTime(int goalstate, int number, float avoidtime) {
	syscall_(BOTLIB_AI_SET_AVOID_GOAL_TIME, goalstate, number, PASSFLOAT(avoidtime));
}

void Q3SysCall::BotInitLevelItems(void) {
	syscall_(BOTLIB_AI_INIT_LEVEL_ITEMS);
}

void Q3SysCall::BotUpdateEntityItems(void) {
	syscall_(BOTLIB_AI_UPDATE_ENTITY_ITEMS);
}

int Q3SysCall::BotLoadItemWeights(int goalstate, char *filename) {
	return syscall_(BOTLIB_AI_LOAD_ITEM_WEIGHTS, goalstate, filename);
}

void Q3SysCall::BotFreeItemWeights(int goalstate) {
	syscall_(BOTLIB_AI_FREE_ITEM_WEIGHTS, goalstate);
}

void Q3SysCall::BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child) {
	syscall_(BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC, parent1, parent2, child);
}

void Q3SysCall::BotSaveGoalFuzzyLogic(int goalstate, char *filename) {
	syscall_(BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC, goalstate, filename);
}

void Q3SysCall::BotMutateGoalFuzzyLogic(int goalstate, float range) {
	syscall_(BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC, goalstate, range);
}

int Q3SysCall::BotAllocGoalState(int state) {
	return syscall_(BOTLIB_AI_ALLOC_GOAL_STATE, state);
}

void Q3SysCall::BotFreeGoalState(int handle) {
	syscall_(BOTLIB_AI_FREE_GOAL_STATE, handle);
}

void Q3SysCall::BotResetMoveState(int movestate) {
	syscall_(BOTLIB_AI_RESET_MOVE_STATE, movestate);
}

void Q3SysCall::BotAddAvoidSpot(int movestate, vec3_t origin, float radius, int type) {
	syscall_(BOTLIB_AI_ADD_AVOID_SPOT, movestate, origin, PASSFLOAT(radius), type);
}

void Q3SysCall::BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags) {
	syscall_(BOTLIB_AI_MOVE_TO_GOAL, result, movestate, goal, travelflags);
}

int Q3SysCall::BotMoveInDirection(int movestate, vec3_t dir, float speed, int type) {
	return syscall_(BOTLIB_AI_MOVE_IN_DIRECTION, movestate, dir, PASSFLOAT(speed), type);
}

void Q3SysCall::BotResetAvoidReach(int movestate) {
	syscall_(BOTLIB_AI_RESET_AVOID_REACH, movestate);
}

void Q3SysCall::BotResetLastAvoidReach(int movestate) {
	syscall_(BOTLIB_AI_RESET_LAST_AVOID_REACH,movestate );
}

int Q3SysCall::BotReachabilityArea(vec3_t origin, int testground) {
	return syscall_(BOTLIB_AI_REACHABILITY_AREA, origin, testground);
}

int Q3SysCall::BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target) {
	return syscall_(BOTLIB_AI_MOVEMENT_VIEW_TARGET, movestate, goal, travelflags, PASSFLOAT(lookahead), target);
}

int Q3SysCall::BotPredictVisiblePosition(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target) {
	return syscall_(BOTLIB_AI_PREDICT_VISIBLE_POSITION, origin, areanum, goal, travelflags, target);
}

int Q3SysCall::BotAllocMoveState(void) {
	return syscall_(BOTLIB_AI_ALLOC_MOVE_STATE);
}

void Q3SysCall::BotFreeMoveState(int handle) {
	syscall_(BOTLIB_AI_FREE_MOVE_STATE, handle);
}

void Q3SysCall::BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove) {
	syscall_(BOTLIB_AI_INIT_MOVE_STATE, handle, initmove);
}

int Q3SysCall::BotChooseBestFightWeapon(int weaponstate, int *inventory) {
	return syscall_(BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON, weaponstate, inventory);
}

void Q3SysCall::BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo) {
	syscall_(BOTLIB_AI_GET_WEAPON_INFO, weaponstate, weapon, weaponinfo);
}

int Q3SysCall::BotLoadWeaponWeights(int weaponstate, char *filename) {
	return syscall_(BOTLIB_AI_LOAD_WEAPON_WEIGHTS, weaponstate, filename);
}

int Q3SysCall::BotAllocWeaponState(void) {
	return syscall_(BOTLIB_AI_ALLOC_WEAPON_STATE);
}

void Q3SysCall::BotFreeWeaponState(int weaponstate) {
	syscall_(BOTLIB_AI_FREE_WEAPON_STATE, weaponstate);
}

void Q3SysCall::BotResetWeaponState(int weaponstate) {
	syscall_(BOTLIB_AI_RESET_WEAPON_STATE, weaponstate);
}

int Q3SysCall::GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child) {
	return syscall_(BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION, numranks, ranks, parent1, parent2, child);
}

int Q3SysCall::PCLoadSource(const char *filename) {
	return syscall_(BOTLIB_PC_LOAD_SOURCE, filename);
}

int Q3SysCall::PCFreeSource(int handle) {
	return syscall_(BOTLIB_PC_FREE_SOURCE, handle);
}

int Q3SysCall::PCReadToken(int handle, pc_token_t *pc_token) {
	return syscall_(BOTLIB_PC_READ_TOKEN, handle, pc_token);
}

int Q3SysCall::PCSourceFileAndLine(int handle, char *filename, int *line) {
	return syscall_(BOTLIB_PC_SOURCE_FILE_AND_LINE, handle, filename, line);
}

void Q3SysCall::MemSet(void* dest, int c, size_t count) {
	syscall_(G_MEMSET, dest, c, count);
}

void Q3SysCall::MemCpy(void *dest, const void *src, size_t count) {
	syscall_(G_MEMCPY, dest, src, count);
}

char *Q3SysCall::StringNCopy(char *strDest, const char *strSource, size_t count) {
	return (char *)syscall_(G_STRNCPY, strDest, strSource, count);
}

float Q3SysCall::Sin(float value) {
	int temp;
	temp = syscall_(G_SIN, PASSFLOAT(value));
	return (*(float*)&temp);
}

float Q3SysCall::Cos(float value) {
	int temp;
	temp = syscall_(G_COS, PASSFLOAT(value));
	return (*(float*)&temp);
}

float Q3SysCall::ATan2(float a, float b) {
	int temp;
	temp = syscall_(G_ATAN2, PASSFLOAT(a), PASSFLOAT(b));
	return (*(float*)&temp);
}

float Q3SysCall::Sqrt(float value) {
	int temp;
	temp = syscall_(G_SQRT, PASSFLOAT(value));
	return (*(float*)&temp);
}

void Q3SysCall::MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]) {
	syscall_(G_MATRIXMULTIPLY, in1, in2, out);
}

void Q3SysCall::AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up) {
	syscall_(G_ANGLEVECTORS, angles, forward, right, up);
}

void Q3SysCall::PerpendicularVector(vec3_t dst, const vec3_t src) {
	syscall_(G_PERPENDICULARVECTOR, dst, src);
}

float Q3SysCall::Floor(float value) {
	int temp;
	temp = syscall_(G_FLOOR, PASSFLOAT(value));
	return (*(float*)&temp);
}

float Q3SysCall::Ceil(float value) {
	int temp;
	temp = syscall_(G_CEIL, PASSFLOAT(value));
	return (*(float*)&temp);
}
