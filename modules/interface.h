#define RS_AC_UCMD_HISTORY_LEN 128

typedef int		fileHandle_t;
typedef enum {qfalse, qtrue}	qboolean;

#ifndef WIN32
typedef char byte;
#endif
typedef struct usercmd_s {
	int				serverTime;
	int				angles[3];
	int 			buttons;
	byte			weapon;           // weapon
	signed char	forwardmove, rightmove, upmove;
} usercmd_t;

typedef struct {
	char name[255];
	char author[255];
	char description[1024];
	char usage[1024];
} ModuleInfo;


typedef enum {
	RS_AC_LOWMOVEUP,
	RS_AC_MINUS128,
	RS_AC_AUTOJUMP
} antiCheat_t;

// Changes to this struct require a rebuild of all modules
typedef struct {
	char name[255]; /* includes colors */
	int state; /* like "CLIENT_ACTIVE" */
	void *ps;
	char userinfo[2048]; /* userinfo string */
	int currtime;

	/* for the module system */
	char **cmd_argv; /* say comand parameters */
	int cmd_progress; /* used for locking the command system */
	int userid; /* link to account */

	/* defrag specific */
	int old_timerflags;
	int old_pm_type;
	int mstime;


	/* AntiCheat stuff */
	usercmd_t cmd[RS_AC_UCMD_HISTORY_LEN]; // usercommands
	int currentUserCmd;
	int CheatFrameCount[8];

	/* prevent misbehaviour */
	int floodprogress;
	char ip[3*4+3+1]; /* 000.000.000.000\n */
	unsigned int stalled;

	/* ServerDemo stuff */
	char demoname[255];
	int demorecording;
	qboolean saveDemo;
	int ServerTimeOnFinish; /* servertime when saving demo */
	int timerStartCount; /* Used for detecting timer resets */
	fileHandle_t demofile;
	int sequence;
} rs_client;

