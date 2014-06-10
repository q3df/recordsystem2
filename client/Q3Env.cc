#include "Q3Env.h"

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

Q3Env *gQ3Env = NULL;

void InitQ3Env() {
	gQ3Env = new Q3Env();
}
