#ifndef SERVER_RECORDSYSTEMDAEMON_H_
#define SERVER_RECORDSYSTEMDAEMON_H_

#include "Console.h"
#include <q3df_api.pb.h>
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>
#include <cstdarg>
#include <pthread.h>
#include <map>

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

#ifdef WIN32
#	include "ConsoleWin32.h"
#else
#	include <unistd.h>
#	include "ConsoleTty.h"
#	define Sleep(x) usleep(x*1000)
#endif

extern Console *gConsole;
extern ProtobufOnceType gEnvQ3dfInitOnce;
extern ::google::protobuf::rpc::Env *gEnvQ3df;

const char *va( const char *format, ... );

typedef std::map<string, string> SettingsMap;
typedef std::map<string, string>::iterator SettingsMapIterator;
extern SettingsMap gSettings;

#endif // SERVER_RECORDSYSTEMDAEMON_H_
