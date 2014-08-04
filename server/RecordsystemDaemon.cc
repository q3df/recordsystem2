// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RecordsystemDaemon.h"

#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

#include "Q3dfApiImpl.h"
#include "ClientMap.h"

#include "../client/StringTokenizer.h"

#include <mysql_public_iface.h>

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

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

int main(int argc, char **argv) {
	Console::Init();
	Q3dfEnv::Init();

	sql::Connection *con = NULL;
	static sql::Driver *driver = NULL;

	bool startupOk = false;

	const sql::SQLString mysql_hostname(argc >= 2 ? argv[1] : "localhost");
	const sql::SQLString mysql_username(argc >= 3 ? argv[2] : "root");
	const sql::SQLString mysql_password(argc >= 4 ? argv[3] : "root");
	const sql::SQLString mysql_database(argc >= 5 ? argv[4] : "USE db_q3df");

	try {
		driver = sql::mysql::get_driver_instance();
		/* Using the Driver to create a connection */
		con = driver->connect(mysql_hostname, mysql_username, mysql_password);

		std::auto_ptr< sql::Statement > stmt(con->createStatement());
		stmt->execute(mysql_database);
		startupOk = true;
		gConsole->PrintInfo("connected to database: '%s'\n", mysql_hostname.c_str());
		
	} catch (sql::SQLException &e) {
		gConsole->PrintError("can't connect to database: '%s'\n", e.what());
		con = NULL;
		driver = NULL;
		startupOk = false;
	}

	if(startupOk) {
		try {
			std::auto_ptr< sql::Statement > stmt(con->createStatement());
			/* Fetching again but using type convertion methods */
			std::auto_ptr< sql::ResultSet > res(stmt->executeQuery("SELECT * FROM q3_servers ORDER BY id"));
			gConsole->Print("Load apikeys of all servers\n");
			gConsole->Print("---------------------------\n");
			while (res->next()) {
				gConsole->Print("  - get apikey for '%s'\n", res->getString("name").c_str());
				string key(va("apikey-%i", res->getInt("id")));
				gSettings[key] = string(res->getString("apikey").c_str());
			}
		} catch (sql::SQLException &e) {
			gConsole->PrintError("could not load servr api keys: '%s'\n", e.what());
		}

		Server server(gEnvQ3df);
		server.AddService(new Q3dfApiImpl(gConsole), true);
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

	if(con != NULL) {
		con->close();
		delete con;
	}

	ClientMap::Dispose();
	Q3dfEnv::Dispose();
	Console::Dispose();



	return 0;
}
