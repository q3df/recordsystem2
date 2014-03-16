#ifndef SERVER_CONSOLETTY_H_
#define SERVER_CONSOLETTY_H_

#include "Console.h"

#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>

typedef struct {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
} field_t;

// This is somewhat of aduplicate of the graphical console history
// but it's safer more modular to have our own here
#define CON_HISTORY 32
#define TTY_CONSOLE_PROMPT "tty]"

extern "C" {
	const char *va( const char *format, ... );
}

class ConsoleTty : public Console {
public:
	ConsoleTty();
	~ConsoleTty();

	virtual char *Input();
	virtual void Print(const char *msg);
	void Init();

protected:
	virtual void Show();
	virtual void Hide();

private:
	void FieldClear(field_t *edit);
	void FlushIn();
	void Back();
	void HistAdd(field_t *field);
	field_t *HistPrev();
	field_t *HistNext();
	void AnsiColorPrint(const char *msg);

	bool stdinIsATTY_;
	bool stdin_active_;

	// general flag to tell about tty console mode
	bool ttycon_on_;
	int ttycon_hide_;
	int ttycon_show_overdue_;

	// some key codes that the terminal may be using, initialised on start up
	int TTY_erase_;
	int TTY_eof_;
	struct termios TTY_tc_;
	field_t TTY_con_;

	field_t ttyEditLines_[CON_HISTORY];
	int hist_current_;
	int hist_count_;
};

#endif // SERVER_CONSOLETTY_H_
