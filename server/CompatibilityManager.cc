#include "CompatibilityManager.h"
#include "Console.h"


#ifdef WIN32
#	include "dirent.h"
#else
#	include <dirent.h>
#endif

CompatibilityManager::CompatibilityManager() {

}

CompatibilityManager::~CompatibilityManager() {

}



extern "C" {
	int sqlNum = 0;	
	struct modules *ModuleList = NULL;
	struct commands *CommandList = NULL;
	
	Function global_table[] = {
		(Function) _AddCommand,
		(Function) _SVSendServerCommand,
		(Function) printf,
		(Function) _SqlConnect,
		(Function) _SqlQuery,
		(Function) _SqlGetQuery,
		(Function) _SqlFetchRow,
		(Function) _SqlFreeResult,
		(Function) _SqlClose,
		(Function) _GetServerId,
		(Function) _InfoValueForKey,
		(Function) _GetClientNum,
		(Function) _GetCurrentMap,
		(Function) _GetCurrentPhysic,
		(Function) _GetCurrentMode,
		(Function) _SVSendConsoleCommand
	};

	void *_SqlConnect(MYSQL *sqlcon) {
		void *res;
		int timeout = 15;
				
		mysql_init(sqlcon);
		mysql_options(sqlcon, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&timeout);

		if(strlen(MysqlCon.user) <= 0 || strlen(MysqlCon.passwd) <= 0 || strlen(MysqlCon.host) <= 0 || strlen(MysqlCon.db) <= 0) {
			gConsole->PrintError("no mysqlconnection\n");
			return NULL;
		}

		res = mysql_real_connect(sqlcon, MysqlCon.host, MysqlCon.user, MysqlCon.passwd, MysqlCon.db, 3306, NULL, CLIENT_COMPRESS);

		if(mysql_errno(sqlcon) != 0) {
			gConsole->PrintError("-> sql_connect() ERROR: %s\n", mysql_error(sqlcon));
			return NULL;
		}

		return res;
	}

	MYSQL_ROW _SqlFetchRow(MYSQL_RES *results) {
		MYSQL_ROW fetch; // typedef char **MYSQL_ROW;
		fetch = mysql_fetch_row(results); // NULL and  NOT NULL are valid
		return fetch;
	}

	void _SqlFreeResult(MYSQL_RES *result) {
		mysql_free_result(result);
	}

	int _SqlQuery (MYSQL *sqlcon, const char *fmt, ...) {
		va_list argptr;
		char query[MAX_SQLQUERY_STRING];

		/* build query (think printfgen) */
		va_start (argptr,fmt);
		vsnprintf (query, sizeof(query), fmt, argptr);
		va_end (argptr);

		/* execute query  */
		mysql_query(sqlcon, query);

		if(mysql_errno(sqlcon) != 0) {
			gConsole->PrintError("-> sql_query() ERROR: %s\n", mysql_error(sqlcon));
			return 0;
		}

		return 1;
	}

	MYSQL_RES *_SqlGetQuery(MYSQL *sqlcon, const char *fmt, ...) {
		char query[MAX_SQLQUERY_STRING];
		va_list argptr;
		MYSQL_RES *res;

		/* build query (think printfgen) */
		va_start (argptr,fmt);
		vsnprintf (query, sizeof(query), fmt, argptr);
		va_end (argptr);


		if(_SqlQuery(sqlcon, "%s", query) == 1) {
			res = mysql_store_result(sqlcon);
			if(res == NULL) {
				gConsole->PrintError("-> sql_get_query() ERROR: %s\n", mysql_error(sqlcon));
				gConsole->PrintInfo("-> query: ^7\"^1%s^7\"\n", query);
			}

			return res;
		} else {
			return NULL;
		}
	}

	void _SqlClose(MYSQL *sqlcon) {
		mysql_close(sqlcon);
	}

	void _SVSendConsoleCommand(int exec_when, const char *text ) {
		gConsole->PrintError("NOT IMPLEMENTED: _SVSendConsoleCommand\n");
	}

	void _SVSendServerCommand( rs_client *cl, const char *format, ...) {
		va_list argptr;
		char str[1024];
		char temp_str[4048];

		memset(&temp_str, 0, sizeof(temp_str));

		va_start(argptr, format);
		vsnprintf(str, 1024, format, argptr);
		va_end(argptr);
		int i = 7;
		int z = 0;
		int Pos = 0;

		if(strncmp(str, "chat", 4) == 0)
			Pos=6;
		else
			Pos=7;

		for(i=Pos; i<strlen(str); i++) {
			if(i == strlen(str)-1) break;

			/*if(str[i] == '\n') {
				temp_str[z] = '\\';

				z++;
				temp_str[z] = 'n';
			} else {*/
				temp_str[z] = str[i];
			//}

			z++;
		}

		gConsole->Print(temp_str);
	}

	char *_InfoValueForKey( const char *s, const char *key ) {
		gConsole->PrintError("NOT IMPLEMENTED: _InfoValueForKey\n");
		return "";
	}

	char *_GetCurrentMap( void ) {
		return "cpmjump";
	}

	int _GetCurrentPhysic( void ) {
		return 1;
	}

	int _GetCurrentMode( void ) {
		return -1;
	}

	int _GetServerId( void ) {
		return 1;
	}

	int _GetClientNum(rs_client *cl) {
		return 1;
	}

	void _AddCommand(ModuleInfo *modinfo, char *cmd, void *(*f)(void *)) {
		struct commands *me = NULL;

		if(CommandList == NULL) {
			if( (CommandList = (struct commands*) malloc( sizeof(struct commands) )) == NULL) {
				gConsole->PrintError("-> AddCommand() ERROR: malloc struct CommandList FAILED\n");
				return;
			}

			CommandList->info = modinfo;
			snprintf(CommandList->cmd, sizeof(CommandList->cmd), "%s", cmd);
			CommandList->func = f;

			CommandList->next = NULL;
			CommandList->prev = NULL;
		}
		else {
			me = CommandList;
			while(me->next != NULL)
				me = me->next;

			if( (me->next = (struct commands*) malloc( sizeof(struct commands) )) == NULL) {
				gConsole->PrintError("AddCommand() ERROR: malloc struct CommandList FAILED\n");
				return;
			}

			me->next->prev = me;
			me = me->next;

			me->info = modinfo;
			snprintf(me->cmd, sizeof(me->cmd), "%s", cmd);
			me->func = f;

			me->next = NULL;
		}

		gConsole->PrintInfo("module '%s' loaded cmd: '%s'\n", modinfo->name, cmd);
		return;
	}

	void DelCommand(char *cmd_name) {
		struct commands *del = CommandList;

		while(del != NULL) {
			if(!strcmp(del->info->name, cmd_name) || !strcmp(del->cmd, cmd_name)) {
				/* First Element ? */
				if( (del->next == NULL) && (del->prev == NULL) ) {
					//printf("INFO: unload '%s' cmd: '%s'\n", del->info->name, del->cmd);
					free(del);
					CommandList = NULL;
					return;
				} else if( (del->prev == NULL) && (del->next != NULL) ) { /* it's the first element but not the only one */
					//printf("INFO: unload '%s' cmd: '%s'\n", del->info->name, del->cmd);
					del->next->prev = NULL;
					CommandList = del->next; // del->next has to point to the beginning of the list
					free(del);
					DelCommand(cmd_name);
					return;
				}

				/* middle Element ? */
				if( (del->next != NULL) && (del->prev != NULL) ) {
					//printf("INFO: unload '%s' cmd: '%s'\n", del->info->name, del->cmd);
					del->prev->next = del->next;
					del->next->prev = del->prev;
					free(del);
					DelCommand(cmd_name);
					return;
				}

				/* last Element ? */
				if( (del->next == NULL) && (del->prev != NULL) ) {
					//printf("INFO: unload '%s' cmd: '%s'\n", del->info->name, del->cmd);
					del->prev->next = NULL; // the prior element is going to be the last.
					free(del);
					DelCommand(cmd_name);
					return;
				}
			}
			del = del->next;
		}

		//printf("-> DelCommand() no element deleted...\n");
	}

	int Add_to_ModuleList(ModuleInfo *modinfo, void *h, ModuleInfo *(*f)(Function *)) {
		struct modules *tmp = NULL;
		int i = 2;

		if(ModuleList == NULL) {
			if( (ModuleList = (struct modules*) malloc( sizeof(struct modules) )) == NULL) {
				gConsole->PrintError("Add_to_ModuleList() ERROR: malloc struct ModuleList FAILED\n");
				return -1;
			}

			ModuleList->info = modinfo;
			ModuleList->handle = h;

			ModuleList->func = f;

			ModuleList->next = NULL;
			ModuleList->prev = NULL;
		}
		else {
			tmp = ModuleList;
			while(tmp->next != NULL) {
				tmp = tmp->next;
				i++;
			}

			if( (tmp->next = (struct modules*) malloc( sizeof(struct modules) )) == NULL) {
				gConsole->PrintError("Add_to_ModuleList() ERROR: malloc struct ModuleList FAILED\n");
				return -1;
			}

			tmp->next->prev = tmp;
			tmp = tmp->next;

			tmp->info = modinfo;
			tmp->handle = h;

			tmp->func = f;

			tmp->next = NULL;
		}

		return 0;
	}

	/* exclusively called by UnloadModule() */
	void Del_from_ModuleList(struct modules *del) {
		/* First Element ? */
		if( (del->next == NULL) && (del->prev == NULL) ) {
			free(del);
			ModuleList = NULL;
			return;
		} else if( (del->prev == NULL) && (del->next != NULL) ) { /* it's the first element but not the only one */
			del->next->prev = NULL;
			ModuleList = del->next; // set del->next to the beginnig of the list
			free(del);
			return;
		}

		/* middle Element ? */
		if( (del->next != NULL) && (del->prev != NULL) ) {
			del->prev->next = del->next;
			del->next->prev = del->prev;
			free(del);
			return;
		}

		/* last Element ? */
		if( (del->next == NULL) && (del->prev != NULL) ) {
			del->prev->next = NULL; // the prior element is going to be the last.
			free(del);
			return;
		}

		gConsole->PrintError("-> Del_from_ModuleList() no element deleted\n");
	}

	void OldUnloadModule() {
		struct modules *me = ModuleList;
		DelCommand(me->info->name);
		Sys_UnloadLibrary(me->handle);
		Del_from_ModuleList(me);
	}

	void OldLoadModule() {
		void *handle;
		ModuleInfo *(*f)(Function *); // function pointer
		char *error;

		char mod_name[60];
		char startfunc[20];
		ModuleInfo *modinfo = NULL;

		snprintf(mod_name, sizeof(mod_name), "modules.%s", Sys_LibraryExt);

		handle = Sys_LoadLibrary(mod_name); // open lib
		if (!handle) { // could not load
			gConsole->PrintError("LoadModule() Module: 'modules.dll' Error: '%s'\n", Sys_LibraryError());
			return;
		}

		Sys_LibraryError(); /* Clear any existing error */

		snprintf(startfunc, sizeof(startfunc), "modules_start");

		*(void **) (&f) = Sys_LoadFunction(handle, startfunc); // set functions pointer to address of searched function setzen
		if ((error =  Sys_LibraryError()) != NULL || f == NULL)  {
			gConsole->PrintError("-> LoadModule() 'modules.dll' could not find 'modules_start' function in module...\n");
			Sys_UnloadLibrary(handle);
			return;
		}

		// start module
		modinfo = (*f)(global_table);

		if(Add_to_ModuleList(modinfo, handle, *f) == -1)
			DelCommand("modules");
	}

	void FreeParam(char **temp) {
		int i = 0;

		if(temp != NULL) {
			while(temp[i] != NULL) {
				free(temp[i]);
				i++;
			}

			free(temp);
		}
	}

	struct commands *GetUserCmd(StringTokenizer *cmdline) {
		int element = 0;
		struct commands *me = CommandList;

		while(me != NULL) {
			if(!strcmp(me->cmd, cmdline->Argv(0))) {
				me->tmp = element;
				return me;
			}

			me = me->next;
		}

		return NULL;
	}

	time_t floodtime;
	rs_client *_GetPlayerByNum(int clientNum);

	int CreateParamArray(char ***_cmd_argv, int t, StringTokenizer *cmdline) {
		int param_count = cmdline->Argc();
		int param_len = 0;
		char param[255];
		memset(&param, 0, sizeof(param));
		int i = 0;

		if (param_count > 0) {
			if(t > 0) {
				*_cmd_argv = (char**) malloc( sizeof(char*) * (param_count)); // create a pointer array for the parameter strings
				(*_cmd_argv)[param_count-1] = NULL; // last element has to be NULL so we can identify it.
			}else{
				*_cmd_argv = (char**) malloc( sizeof(char*) * (param_count+1)); // create a pointer array for the parameter strings
				(*_cmd_argv)[param_count] = NULL; // last element has to be NULL so we can identify it.
			}

			if(_cmd_argv == NULL) {gConsole->PrintError("struct to array: Could not allocate memory for pointerarray\n"); return -1;}

			for(i=0; i<((t>0) ? param_count-1 : param_count); i++) {
				if(t>0)
					snprintf(param, sizeof(param), "%s", cmdline->Argv(i+1));
				else
					snprintf(param, sizeof(param), "%s", cmdline->Argv(i));

				param_len = strlen(param);
				(*_cmd_argv)[i] = NULL;
				(*_cmd_argv)[i] = (char*) malloc( sizeof(char) * param_len +1); // getting space for the parameters + a '\0'

				if((*_cmd_argv)[i] == NULL) {gConsole->PrintError("struct to array: Could not allocate memory for arraystring\n"); return -1;}

				strncpy((*_cmd_argv)[i], param, param_len);
				(*_cmd_argv)[i][param_len] = '\0';
			}
		}
		return 0;
	}

	int _OldModuleFindCommand(int clientNum, StringTokenizer *cmdline) {
		rs_client cl;
		memset(&cl, 0, sizeof(rs_client));
		strcpy(cl.name, "^1console^7");
		struct commands *me = GetUserCmd(cmdline);
		int unixTime = time(0); // in seconds

		if(me != NULL) {
			if(cl.cmd_progress == 0) {
				CreateParamArray(&cl.cmd_argv, me->tmp, cmdline);
				if(cl.cmd_argv != NULL) {
					if(floodtime > unixTime) { FreeParam(cl.cmd_argv); return 1; }

					cl.cmd_progress = 1;
					floodtime = unixTime+2; // add two seconds
					me->func(&cl);
				}
			}
			return 1;
		}

		return 0;
	}

	void *M_CMD_help( void *cl );
	ModuleInfo help_info;

	void OldLoadHelpCommand( void ) {
		strcpy(help_info.name, "help");
		strcpy(help_info.author, "^1c^2h^3i^4c^5k^6e^7n");
		strcpy(help_info.description, "print this or detail help of module");
		strcpy(help_info.usage, "!help cmd");
		_AddCommand(&help_info, "!help", M_CMD_help);
	}

	void *M_CMD_help( void *tmp ) {
		rs_client *cl = (rs_client *)tmp;

		int i = 0, show = 0;
		struct commands *me = CommandList;

		while(cl->cmd_argv[i] != NULL) {
			if(strcmp(cl->cmd_argv[i], "show"))
				show = 1;
			i++;
		}

		if(cl->cmd_argv[1] != NULL) {
			while(me != NULL) {
				if(!strcmp(cl->cmd_argv[1], me->cmd+1) || !strcmp(cl->cmd_argv[1], me->cmd)) {
					_SVSendServerCommand( (show ? cl : NULL), "print \"\n %s ^1(^7Author: %s^1)^7\n^5----------------------------------^7\n\"", me->cmd, me->info->author);
					_SVSendServerCommand( (show ? cl : NULL), "print \"^1Description:^7\n %s^7\n\n\"", me->info->description);
					_SVSendServerCommand( (show ? cl : NULL), "print \"^1usage:^7\n %s^7\n\n\"", me->info->usage);
				}
				me = me->next;
			}
		}
		else{
			_SVSendServerCommand( (show ? cl : NULL), "print \"^7\nInformation about commands...\n^5----------------------------------^7\n\"");

			while(me != NULL) {
				_SVSendServerCommand( (show ? cl : NULL), "print \" %-10s ^5-> ^1(^7Author: %s^1)^7\n\"", me->cmd, me->info->author);
				me = me->next;
			}
		}

		i=0;
		while(cl->cmd_argv[i] != NULL) {
			free(cl->cmd_argv[i]);
			i++;
		}
		free(cl->cmd_argv);

		cl->cmd_progress = 0;
		return NULL;
	}
}
