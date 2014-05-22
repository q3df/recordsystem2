#ifndef SERVER_COMPATIBILITYMANAGER_H__
#define SERVER_COMPATIBILITYMANAGER_H__

#include <map>
#include <list>
#include "../client/StringTokenizer.h"

#define RS_AC_UCMD_HISTORY_LEN 128
#define MAX_SQLQUERY_STRING 4096
#define REC_MAX_SQLQUERY_STRING 4096
#define SQL_KEY_LEN 367

static struct {
	char host[50];
	char user[50];
	char passwd[50];
	char db[50];
} MysqlCon;

#ifdef _WIN32
#   include <windows.h>
#   define Sys_LoadLibrary(f) (void*)LoadLibrary(f)
#   define Sys_UnloadLibrary(h) FreeLibrary((HMODULE)h)
#   define Sys_LoadFunction(h,fn) (void*)GetProcAddress((HMODULE)h,fn)
#   define Sys_LibraryError() NULL
#	define Sys_LibraryExt "dll"
#	define snprintf _snprintf
#else
#   include <dlfcn.h>
#   define Sys_LoadLibrary(f) dlopen(f,RTLD_NOW)
#   define Sys_UnloadLibrary(h) dlclose(h)
#   define Sys_LoadFunction(h,fn) dlsym(h,fn)
#   define Sys_LibraryError() dlerror()
#	define Sys_LibraryExt "so"
#endif

class CompatibilityManager {
private:

public:
	CompatibilityManager();
	~CompatibilityManager();
};

extern "C" {
	#include <mysql.h>

	typedef void (*Function)( void );

	#include "../modules/interface.h"

	struct modules {
		void *handle;
		ModuleInfo *info;
		ModuleInfo *(*func)(Function *);
		struct modules *next;
		struct modules *prev;
	};

	struct commands {
		ModuleInfo *info;
		char cmd[255];
		void *(*func)(void *);
		struct commands *next;
		struct commands *prev;
		int tmp;
	};

	void OldLoadHelpCommand( void );
	void _AddCommand(ModuleInfo *modinfo, char *cmd, void *(*f)(void *));
	void _SVSendServerCommand( rs_client *cl, const char *format, ...);
	void _RSPrintf(const char *format, ...);
	void *_SqlConnect(MYSQL *sqlcon);
	int _SqlQuery(MYSQL *sqlcon, const char *fmt, ...);
	MYSQL_RES *_SqlGetQuery(MYSQL *sqlcon, const char *fmt, ...);
	MYSQL_ROW _SqlFetchRow(MYSQL_RES *results);
	void _SqlFreeResult(MYSQL_RES *result);
	void _SqlClose(MYSQL *sqlcon);
	int _GetServerId();
	char *_InfoValueForKey( const char *s, const char *key );
	int _GetClientNum(rs_client *cl);
	char *_GetCurrentMap();
	int _GetCurrentPhysic();
	int _GetCurrentMode();
	void _SVSendConsoleCommand(int exec_when, const char *text);
	int _OldModuleFindCommand(int clientNum, StringTokenizer *cmdline);
	void OldUnloadModule();
	void OldLoadModule();
}

#endif // SERVER_COMPATIBILITYMANAGER_H__