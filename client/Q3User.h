#ifndef CLIENT_Q3USER_H_
#define CLIENT_Q3USER_H_

class Recordsystem;

#include "Q3SysCall.h"
#include "Quake3.h"
#include <string>

/** !! NOT THREAD SAFE !! */

class Q3User {
friend class Recordsystem;
public:
	Q3User(int playernum);
	~Q3User();

	int GetPlayernum();
	playerState_t *Playerstate();

	inline clientState_t GetState() { return state_; }
	inline Q3User *SetState(clientState_t state) { state_ = state; return this; }

	inline const char *GetUserInfoString() { return userInfo_; }
	inline Q3User *SetUserInfoString(const char *infoString) { strncpy(userInfo_, infoString, sizeof(userInfo_)); return this; };

	const char *GetUserInfo(const char *key );
	Q3User *SetUserInfo(const char *key, const char *value);

	inline int GetUserId() { return userId_; }
	inline Q3User *SetUserId(int userId) { userId_ = userId; return this; }

	Q3User *Reset();

private:
	char userInfo_[MAX_INFO_STRING];
	int playernum_;
	int userId_;
	clientState_t state_;

};
#endif // CLIENT_Q3USER_H_

