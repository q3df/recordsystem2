#ifndef CLIENT_Q3SYSCALL_H_
#define CLIENT_Q3SYSCALL_H_

#include <string>
#include "Q3Event.h"
#include "Quake3.h"

using namespace std;

class Q3SysCall {
public:
	Q3SysCall(syscall_t syscall);
	~Q3SysCall();

	void Printf(const char *fmt);

	// q3df specific start
	void Print(const char *fmt);
	void PrintWarning(const char *fmt);
	void PrintError(const char *fmt);
	// q3df specific stop

	void Error(const char *fmt);
	int Milliseconds(void);
	int Argc(void);
	void Argv(int n, char *buffer, int bufferLength);
	int FSFOpenFile(const char *qpath, fileHandle_t *f, fsMode_t mode);
	void FSRead(void *buffer, int len, fileHandle_t f);
	void FSWrite(const void *buffer, int len, fileHandle_t f);
	void FSFCloseFile(fileHandle_t f);
	int FSGetFileList(const char *path, const char *extension, char *listbuf, int bufsize);
	int FSSeek(fileHandle_t f, long offset, int origin);
	void SendConsoleCommand(int exec_when, const char *text);
	void CvarRegister(vmCvar_t *cvar, const char *var_name, const char *value, int flags);
	void CvarUpdate(vmCvar_t *cvar);
	void CvarSet(const char *var_name, const char *value);
	int CvarVariableIntegerValue(const char *var_name);
	void CvarVariableStringBuffer(const char *var_name, char *buffer, int bufsize);
	void LocateGameData(gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGClient);
	void DropClient(int clientNum, const char *reason);
	void SendServerCommand(int clientNum, const char *text);
	void SetConfigstring(int num, const char *string);
	void GetConfigstring(int num, char *buffer, int bufferSize);
	void GetUserinfo(int num, char *buffer, int bufferSize);
	void SetUserinfo(int num, const char *buffer);
	void GetServerinfo(char *buffer, int bufferSize);
	void SetBrushModel(gentity_t *ent, const char *name);
	void Trace(trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask);
	void TraceCapsule(trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask);
	int PointContents(const vec3_t point, int passEntityNum);
	qboolean InPVS(const vec3_t p1, const vec3_t p2);
	qboolean InPVSIgnorePortals(const vec3_t p1, const vec3_t p2);
	void AdjustAreaPortalState(gentity_t *ent, qboolean open);
	qboolean AreasConnected(int area1, int area2);
	void LinkEntity(gentity_t *ent);
	void UnlinkEntity(gentity_t *ent);
	int EntitiesInBox(const vec3_t mins, const vec3_t maxs, gentity_t **list, int maxcount);
	qboolean EntityContact(const vec3_t mins, const vec3_t maxs, const gentity_t *ent);
	qboolean EntityContactCapsule(const vec3_t mins, const vec3_t maxs, const gentity_t *ent);
	int BotAllocateClient(void);
	void BotFreeClient(int clientNum);
	void GetUsercmd(int clientNum, usercmd_t *cmd);
	qboolean GetEntityToken(char *buffer, int bufferSize);
	int DebugPolygonCreate(int color, int numPoints, vec3_t *points);
	void DebugPolygonDelete(int id);
	int RealTime(qtime_t *qtime);
	void SnapVector(float *v);

	int BotLibSetup(void);
	int BotLibShutdown(void);
	int BotLibVarSet(char *var_name, char *value);
	int BotLibVarGet(char *var_name, char *value, int size);
	int BotLibDefine(char *string);
	int BotLibStartFrame(float time);
	int BotLibLoadMap(const char *mapname);
	int BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);
	int BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);
	int BotGetSnapshotEntity(int clientNum, int sequence);
	int BotGetServerCommand(int clientNum, char *message, int size);
	void BotUserCommand(int clientNum, usercmd_t *ucmd);
	void AASEntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info);
	int AASInitialized(void);
	void AASPresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs);
	float AASTime(void);
	int AASPointAreaNum(vec3_t point);
	int AASPointReachabilityAreaIndex(vec3_t point);
	int AASTraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas);
	int AASBBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas);
	int AASAreaInfo(int areanum, void /* struct aas_areainfo_s */ *info);
	int AASPointContents(vec3_t point);
	int AASNextBSPEntity(int ent);
	int AASValueForBSPEpairKey(int ent, char *key, char *value, int size);
	int AASVectorForBSPEpairKey(int ent, char *key, vec3_t v);
	int AASFloatForBSPEpairKey(int ent, char *key, float *value);
	int AASIntForBSPEpairKey(int ent, char *key, int *value);
	int AASAreaReachability(int areanum);
	int AASAreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags);
	int AASEnableRoutingArea(int areanum, int enable);
	int AASPredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin, int goalareanum, int travelflags, int maxareas, int maxtime, int stopevent, int stopcontents, int stoptfl, int stopareanum);
	int AASAlternativeRouteGoals(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags, void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals, int type);
	int AASSwimming(vec3_t origin);
	int AASPredictClientMovement(void /* struct aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize);
	void EASay(int client, char *str);
	void EASayTeam(int client, char *str);
	void EACommand(int client, char *command);
	void EAAction(int client, int action);
	void EAGesture(int client);
	void EATalk(int client);
	void EAAttack(int client);
	void EAUse(int client);
	void EARespawn(int client);
	void EACrouch(int client);
	void EAMoveUp(int client);
	void EAMoveDown(int client);
	void EAMoveForward(int client);
	void EAMoveBack(int client);
	void EAMoveLeft(int client);
	void EAMoveRight(int client);
	void EASelectWeapon(int client, int weapon);
	void EAJump(int client);
	void EADelayedJump(int client);
	void EAMove(int client, vec3_t dir, float speed);
	void EAView(int client, vec3_t viewangles);
	void EAEndRegular(int client, float thinktime);
	void EAGetInput(int client, float thinktime, void /* struct bot_input_s */ *input);
	void EAResetInput(int client);
	int BotLoadCharacter(char *charfile, float skill);
	void BotFreeCharacter(int character);
	float CharacteristicFloat(int character, int index);
	float CharacteristicBFloat(int character, int index, float min, float max);
	int CharacteristicInteger(int character, int index);
	int CharacteristicBInteger(int character, int index, int min, int max);
	void CharacteristicString(int character, int index, char *buf, int size);
	int BotAllocChatState(void);
	void BotFreeChatState(int handle);
	void BotQueueConsoleMessage(int chatstate, int type, char *message);
	void BotRemoveConsoleMessage(int chatstate, int handle);
	int BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm);
	int BotNumConsoleMessages(int chatstate);
	void BotInitialChat(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7);
	int BotNumInitialChats(int chatstate, char *type);
	int BotReplyChat(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7);
	int BotChatLength(int chatstate);
	void BotEnterChat(int chatstate, int client, int sendto);
	void BotGetChatMessage(int chatstate, char *buf, int size);
	int StringContains(char *str1, char *str2, int casesensitive);
	int BotFindMatch(char *str, void /* struct bot_match_s */ *match, unsigned long int context);
	void BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size);
	void UnifyWhiteSpaces(char *string);
	void BotReplaceSynonyms(char *string, unsigned long int context);
	int BotLoadChatFile(int chatstate, char *chatfile, char *chatname);
	void BotSetChatGender(int chatstate, int gender);
	void BotSetChatName(int chatstate, char *name, int client);
	void BotResetGoalState(int goalstate);
	void BotResetAvoidGoals(int goalstate);
	void BotRemoveFromAvoidGoals(int goalstate, int number);
	void BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal);
	void BotPopGoal(int goalstate);
	void BotEmptyGoalStack(int goalstate);
	void BotDumpAvoidGoals(int goalstate);
	void BotDumpGoalStack(int goalstate);
	void BotGoalName(int number, char *name, int size);
	int BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal);
	int BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal);
	int BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags);
	int BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime);
	int BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal);
	int BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal);
	int BotGetLevelItemGoal(int index, char *classname, void /* struct bot_goal_s */ *goal);
	int BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal);
	int BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal);
	float BotAvoidGoalTime(int goalstate, int number);
	void BotSetAvoidGoalTime(int goalstate, int number, float avoidtime);
	void BotInitLevelItems(void);
	void BotUpdateEntityItems(void);
	int BotLoadItemWeights(int goalstate, char *filename);
	void BotFreeItemWeights(int goalstate);
	void BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
	void BotSaveGoalFuzzyLogic(int goalstate, char *filename);
	void BotMutateGoalFuzzyLogic(int goalstate, float range);
	int BotAllocGoalState(int state);
	void BotFreeGoalState(int handle);
	void BotResetMoveState(int movestate);
	void BotAddAvoidSpot(int movestate, vec3_t origin, float radius, int type);
	void BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags);
	int BotMoveInDirection(int movestate, vec3_t dir, float speed, int type);
	void BotResetAvoidReach(int movestate);
	void BotResetLastAvoidReach(int movestate);
	int BotReachabilityArea(vec3_t origin, int testground);
	int BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target);
	int BotPredictVisiblePosition(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target);
	int BotAllocMoveState(void);
	void BotFreeMoveState(int handle);
	void BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove);
	int BotChooseBestFightWeapon(int weaponstate, int *inventory);
	void BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo);
	int BotLoadWeaponWeights(int weaponstate, char *filename);
	int BotAllocWeaponState(void);
	void BotFreeWeaponState(int weaponstate);
	void BotResetWeaponState(int weaponstate);
	int GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child);
	int PCLoadSource(const char *filename);
	int PCFreeSource(int handle);
	int PCReadToken(int handle, pc_token_t *pc_token);
	int PCSourceFileAndLine(int handle, char *filename, int *line);

	void MemSet(void* dest, int c, size_t count);
	void MemCpy(void *dest, const void *src, size_t count);
	char *StringNCopy(char *strDest, const char *strSource, size_t count);
	float Sin(float value);
	float Cos(float value);
	float ATan2(float a, float b);
	float Sqrt(float value);
	void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
	void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
	void PerpendicularVector(vec3_t dst, const vec3_t src );
	float Floor(float value);
	float Ceil(float value);

	void AddEventHandler(Q3EventHandler *eventItem);
	void RemoveEventHandler(Q3EventHandler *eventItem);

private:
	syscall_t syscall_;
	EventHandlers eventList_;
};

#endif // CLIENT_Q3SYSCALL_H_
