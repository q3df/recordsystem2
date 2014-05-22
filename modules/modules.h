#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#	include <winsock.h>
#	include <windows.h>
#	define snprintf _snprintf
#	define SVN_VERSION "win"
#	define COMPILE_DATE "21-07-2009"
#endif

#ifdef WIN32
#	define snprinf _snprintf
#	define DLLEXPORT __declspec(dllexport)
#	define C_DLLEXPORT extern "C" DLLEXPORT
#	ifdef linux
#		undef linux
#	endif
#else
#	define C_DLLEXPORT
#endif

extern "C" {
//#include <q_shared.h>
#include <mysql.h>

#include "interface.h"

typedef void (*Function)();

/* interface cmd_module => proxy_mod */
/* see rs_modules.h global table for order */
#define AddCommand ((void (*)(ModuleInfo *, char *, void (*)(rs_client *))) global[0])
#define SV_SendServerCommand ( (void (*)(rs_client *, const char *, ...)) global[1])
#define RS_Printf ((void (*)(const char *, ...))global[2])
#define sql_connect ((void *(*)(MYSQL *)) global[3])
#define sql_query ((int (*)(MYSQL *, const char *, ...)) global[4])
#define sql_get_query ((MYSQL_RES *(*)(MYSQL *, const char *, ...)) global[5])
#define sql_fetch_row ((MYSQL_ROW (*)(MYSQL_RES *)) global[6])
#define sql_free_result ((void (*)(MYSQL_RES *)) global[7])
#define sql_close ((void (*)(MYSQL *)) global[8])
#define GetServerId ((int (*)()) global[9])
#define Info_ValueForKey ((char *(*)(const char *, const char *)) global[10])
#define GetClientNum ((int (*)(rs_client *)) global[11])
#define GetCurrentMap ((char *(*)()) global[12])
#define GetCurrentPhysic ((int (*)()) global[13])
#define GetCurrentMode ((int (*)()) global[14])
#define SV_SendConsoleCommand ( (void (*)(int, const char *)) global[15])
#define GetPlayerByNum ((rs_client *(*)(int)) global[16])
#define GetPlayerDefragTime ((int (*)(rs_client *)) global[17])



char *va(char *format, ...);
void print_user_cmd_to_chat( rs_client *cl, int to_all);
void FreeParameterArray(char **temp);
int atoi_dftime(char *dftime, int *i);
int dftime_to_mstime(char *dftime);
char* mstime_to_dftime (int mstime, int opt);
int cleanup(rs_client *cl);
int GetELOEnabled(void);
char * str_clean(char * str);
}