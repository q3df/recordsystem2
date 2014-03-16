#include "Q3User.h"
#include "Recordsystem.h"
#include "Utils.h"

Q3User::Q3User(int playernum) {
	playernum_ = playernum;
	memset(&userInfo_, 0, sizeof(userInfo_));
}

Q3User::~Q3User() {
}

int Q3User::GetPlayernum() {
	return playernum_;
}

playerState_t *Q3User::Playerstate() {
	return gRecordsystem->GetPlayerstate(playernum_);
}

const char *Q3User::GetUserInfo(const char *key) {
	return Utils::InfoValueForKey(userInfo_, key);
}

Q3User *Q3User::SetUserInfo(const char *key, const char *value) {
	Utils::InfoSetValueForKey(userInfo_, key, value);
	return this;
}

Q3User *Q3User::Reset() {
	memset(&userInfo_, 0, sizeof(userInfo_));
	state_ = CLIENT_FREE;
	userId_ = 0;
	uniqueId_ = 0;
	return this;
}
