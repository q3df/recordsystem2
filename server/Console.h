#ifndef SERVER_CONSOLE_H_
#define SERVER_CONSOLE_H_

#include <string>
#include <stdarg.h>

#define	MAXPRINTMSG	4096
#define QCONSOLE_HISTORY 32
#define MAX_EDIT_LINE 256
#define Q_COLOR_ESCAPE	'^'
#define Q_IsColorString(p)	((p) && *(p) == Q_COLOR_ESCAPE && *((p)+1) && isalnum(*((p)+1))) // ^[0-9a-zA-Z]

#define COLOR_BLACK		'0'
#define COLOR_RED		'1'
#define COLOR_GREEN		'2'
#define COLOR_YELLOW	'3'
#define COLOR_BLUE		'4'
#define COLOR_CYAN		'5'
#define COLOR_MAGENTA	'6'
#define COLOR_WHITE		'7'
#define ColorIndex(c)	(((c) - '0') & 0x07)
typedef float vec_t;
typedef vec_t vec4_t[4];
extern vec4_t g_color_table[8];


class Console {
public:
	Console();
	~Console();

	virtual char *Input() = 0;
	virtual void Print(const char *msg) = 0;

protected:
	virtual void Show() = 0;
	virtual void Hide() = 0;

private:

};

#endif // SERVER_CONSOLE_H_
