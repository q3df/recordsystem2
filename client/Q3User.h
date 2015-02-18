#ifndef CLIENT_Q3USER_H_
#define CLIENT_Q3USER_H_

class Recordsystem;

#include "Q3SysCall.h"
#include "Quake3.h"
#include <string>
#include <q3df_api.pb.h>

#ifdef WIN32
#	define snprintf _snprintf
#endif

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

	inline string *GetUniqueId() { return &uniqueId_; }
	inline void SetUniqueId(string uniqueId) { uniqueId_ = uniqueId; }

	inline const char *GetLastQ3dfkey() { return this->lastQ3dfkey_; }
	inline Q3User *SetLastQ3dfKey(const char *q3dfkey) {
		memset(&this->lastQ3dfkey_, 0, sizeof(this->lastQ3dfkey_));
		snprintf(this->lastQ3dfkey_, MAX_INFO_STRING, "%s", q3dfkey);
		this->lastQ3dfkey_[MAX_INFO_STRING-1] = '\0';
		return this;
	}

	Q3User *WriteIdentifier(::service::Identifier *identity);

	Q3User *Reset();

private:
	char userInfo_[MAX_INFO_STRING];
	int playernum_;
	int userId_;
	string uniqueId_;
	char lastQ3dfkey_[MAX_INFO_STRING];
	clientState_t state_;

};
#endif // CLIENT_Q3USER_H_

