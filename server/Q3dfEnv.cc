#include "Q3dfEnv.h"

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

void InitQ3dfEnv() {
	gEnvQ3df = new Q3dfEnv(gConsole);
}
