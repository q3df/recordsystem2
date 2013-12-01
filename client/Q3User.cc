#include "Q3User.h"
#include "Recordsystem.h"

Q3User::Q3User(int playernum) {
	playernum_ = playernum;
}

Q3User::~Q3User() { }

int Q3User::GetPlayernum() {
	return playernum_;
}

playerState_t *Q3User::Playerstate() {
	return gRecordsystem->GetPlayerstate(playernum_);
}
