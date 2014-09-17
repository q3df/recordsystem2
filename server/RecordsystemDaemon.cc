// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RecordsystemDaemon.h"

#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

#include "Q3dfApiImpl.h"
#include "ClientMap.h"

#include "../client/StringTokenizer.h"

#include <Request.hpp>
#include <Response.hpp>
#include <boost/program_options.hpp>
#include <sstream>

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

namespace po = boost::program_options;

SettingsMap gSettings;

extern "C" {
	const char *va( const char *format, ... ) {
		va_list		argptr;
		static char		string[2][32000];	// in case va is called by nested functions
		static int		index = 0;
		char	*buf;

		buf = string[index & 1];
		index++;

		va_start (argptr, format);
		vsprintf (buf, format, argptr);
		va_end (argptr);

		return buf;
	}
}

MysqlPool *gMysqlPool;
static sql::Driver *driver = NULL;

int main(int argc, char **argv) {
	Console::Init();

	po::options_description desc("Allowed options");

	string mysql_hostname;
	string mysql_username;
	string mysql_password;
	string mysql_database;
	int listenPort;
	int mysqlPoolSize;

	desc.add_options()
	  ("help", "produce help message")
	  ("mysql_host",   po::value<string>(&mysql_hostname)->default_value(string("localhost")),  "MySQL host.")
	  ("mysql_user",   po::value<string>(&mysql_username)->default_value(string("root")),  "MySQL username.")
	  ("mysql_passwd", po::value<string>(&mysql_password)->default_value(string("root")),  "MySQL password.")
	  ("mysql_db",     po::value<string>(&mysql_database)->default_value(string("USE db_q3df")), "MySQL database name.")
	  ("api_listen_port", po::value<int>(&listenPort)->default_value(1234), "api listen on a specific port.")
	  ("mysql_pool_size", po::value<int>(&mysqlPoolSize)->default_value(10), "The pool size of the mysql connections.")
	;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

	if (vm.count("help")) {
		std::ostringstream tmpDesc;
		desc.print(tmpDesc);

		gConsole->PrintInfo("Usage: recordsystemd [options]\n");
        gConsole->PrintInfo("%s\n", tmpDesc.str().c_str());
		Console::Dispose();
        return 0;
	}

	Q3dfEnv::Init();

	bool startupOk = false;
	try {
		gConsole->PrintInfo("Initialize MysqlPool to database server '%s' with %i connections...\n", mysql_hostname.c_str(), mysqlPoolSize);
		gMysqlPool = new MysqlPool(mysqlPoolSize, [&mysql_hostname, &mysql_username, &mysql_password, &mysql_database]() -> sql::Connection* {
			if(driver == NULL)
				driver = sql::mysql::get_driver_instance();

			/* Using the Driver to create a connection */
			sql::Connection *con = driver->connect(mysql_hostname, mysql_username, mysql_password);

			std::auto_ptr< sql::Statement > stmt(con->createStatement());
			stmt->execute(mysql_database);

			return con;
		});
		startupOk = true;
	} catch (sql::SQLException &e) {
		gConsole->PrintError("can't connect to database: '%s'\n", e.what());
		driver = NULL;
		startupOk = false;
	}
	
	if(startupOk) {
		sql::Connection *mcon = gMysqlPool->Get();
		try {
			std::auto_ptr< sql::Statement > stmt(mcon->createStatement());
			/* Fetching again but using type convertion methods */
			std::auto_ptr< sql::ResultSet > res(stmt->executeQuery("SELECT * FROM q3_servers ORDER BY id"));
			gConsole->PrintInfo("Load apikeys of all servers\n");
			gConsole->PrintInfo("---------------------------\n");
			while (res->next()) {
				gConsole->PrintInfo("  - get apikey for '%s'\n", res->getString("name").c_str());
				string key(va("apikey-%i", res->getInt("id")));
				gSettings[key] = string(res->getString("apikey").c_str());
			}
		} catch (sql::SQLException &e) {
			gConsole->PrintError("could not load servr api keys: '%s'\n", e.what());
		}

		gMysqlPool->Return(mcon);
		Server server(gEnvQ3df);
		server.AddService(new Q3dfApiImpl(gConsole), true);
		gConsole->PrintInfo("API-Server starting listen on port %i.\n", listenPort);
		server.ListenTCP(1234);

		for(;;) {
			char *cmd = gConsole->Input();
			if(cmd && !strncmp(cmd, "exit", 4)) {
				break;

			} else if(cmd && !strncmp(cmd, "status", 5)) {
				ClientMap::PrintList(gConsole);

			} else if(cmd && !strncmp(cmd, "settingslist", 12)) {
				SettingsMapIterator it;
				gConsole->Print("  Settings-List\n");
				gConsole->Print(" ^3---------------------------------------^7\n");
				for (it=gSettings.begin(); it!=gSettings.end(); ++it)
					gConsole->Print("    %s = '%s'\n", it->first.c_str(), it->second.c_str());

			} else if(cmd && !strncmp(cmd, "set", 3)) {
				StringTokenizer *cmdline = new StringTokenizer(cmd, false);
				if(cmdline->Argc() == 3) {
					string key(cmdline->Argv(1));
					gSettings[key].clear();
					gSettings[key].append(cmdline->Argv(2));
					gConsole->Print("%s='%s' SAVED.\n", cmdline->Argv(1), gSettings[key].c_str());
				}else
					gConsole->PrintError("usage: set <varname> <value>\n");

				delete cmdline;

			} else if(cmd && !strncmp(cmd, "get", 3)) {
				StringTokenizer *cmdline = new StringTokenizer(cmd, false);
				string key(cmdline->Argv(1));
				if(cmdline->Argc() == 2 && gSettings.find(key) != gSettings.end()) {
					gConsole->Print("RESULT: %s='%s'\n", cmdline->Argv(1), gSettings[key].c_str());
				}else
					gConsole->PrintError("RESULT: '%s' not found!\n", cmdline->Argv(1));

				delete cmdline;

			} else if(cmd) {
				StringTokenizer *cmdline = new StringTokenizer(cmd, false);
				// do anything generic for command line plugins ;)
				delete cmdline;

			}

			Conn *conn = server.AcceptNonBlock();
			if(conn) {
				gConsole->PrintInfo("incoming connection from ^3%s^7\n", conn->RemoteIpAdress());
				server.Serve(conn);
			}

			Sleep(10);
		}
	}

	gConsole->PrintInfo(va("Shutingdown now...\n"));
	Sleep(1000);
	
	delete gMysqlPool;

	ClientMap::Dispose();
	Q3dfEnv::Dispose();
	Console::Dispose();
	
	return 0;
}
