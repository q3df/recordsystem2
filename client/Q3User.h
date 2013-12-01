#ifndef CLIENT_Q3USER_H_
#define CLIENT_Q3USER_H_

class Recordsystem;

#include "Q3SysCall.h"
#include "Quake3.h"

class Q3User {
friend class Recordsystem;
public:
	Q3User(int playernum);
	~Q3User();

	int GetPlayernum();
	playerState_t *Playerstate();

	inline clientState_t GetState() { return state_; }
	inline Q3User *SetState(clientState_t state) { state_ = state; return this; }

private:
	int playernum_;
	clientState_t state_;
};
#endif // CLIENT_Q3USER_H_

