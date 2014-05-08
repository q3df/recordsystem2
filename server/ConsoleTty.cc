#include "ConsoleTty.h"
#include <unistd.h>

ConsoleTty *gCon = NULL;

extern "C" {
	static void SigCont(int signum) {
		gCon->Init();
	}
}

ConsoleTty::ConsoleTty() {
	gCon = this;
	ttycon_on_ = false;
	ttycon_hide_ = 0;
	ttycon_show_overdue_ = 0;
	hist_current_ = -1;
	hist_count_ = 0;

	const char* term = getenv("TERM");
	stdinIsATTY_ = isatty(STDIN_FILENO) &&	!(term && (!strcmp(term, "raw") || !strcmp(term, "dumb")));

	Init();

	this->Print("Console initialized...\n");
}

void ConsoleTty::Init() {
	struct termios tc;

	// If the process is backgrounded (running non interactively)
	// then SIGTTIN or SIGTOU is emitted, if not caught, turns into a SIGSTP
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	// If SIGCONT is received, reinitialize console
	signal(SIGCONT, SigCont);

	// Make stdin reads non-blocking
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK );

	if (!stdinIsATTY_)
	{
		printf("tty console mode disabled\n");
		ttycon_on_ = false;
		stdin_active_ = true;
		return;
	}

	FieldClear(&TTY_con_);
	tcgetattr (STDIN_FILENO, &TTY_tc_);
	TTY_erase_ = TTY_tc_.c_cc[VERASE];
	TTY_eof_ = TTY_tc_.c_cc[VEOF];
	tc = TTY_tc_;

	/*
	ECHO: don't echo input characters
	ICANON: enable canonical mode.  This  enables  the  special
	characters  EOF,  EOL,  EOL2, ERASE, KILL, REPRINT,
	STATUS, and WERASE, and buffers by lines.
	ISIG: when any of the characters  INTR,  QUIT,  SUSP,  or
	DSUSP are received, generate the corresponding signal
	*/
	tc.c_lflag &= ~(ECHO | ICANON);

	/*
	ISTRIP strip off bit 8
	INPCK enable input parity checking
	*/
	tc.c_iflag &= ~(ISTRIP | INPCK);
	tc.c_cc[VMIN] = 1;
	tc.c_cc[VTIME] = 0;
	tcsetattr (STDIN_FILENO, TCSADRAIN, &tc);
	ttycon_on_ = true;
	ttycon_hide_ = 1; // Mark as hidden, so prompt is shown in CON_Show
	Show();
}

ConsoleTty::~ConsoleTty() {
	this->Print("Shuting down console...\n");
	if (ttycon_on_) {
		Hide();
		tcsetattr (STDIN_FILENO, TCSADRAIN, &TTY_tc_);
	}

	// Restore blocking to stdin reads
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) & ~O_NONBLOCK);
}

char *ConsoleTty::Input() {
	// we use this when sending back commands
	static char text[MAX_EDIT_LINE];
	int avail;
	char key;
	field_t *history;

	if(ttycon_on_) {
		avail = read(STDIN_FILENO, &key, 1);
		if (avail != -1) {
			// we have something
			// backspace?
			// NOTE TTimo testing a lot of values .. seems it's the only way to get it to work everywhere
			if ((key == TTY_erase_) || (key == 127) || (key == 8)) {
				if (TTY_con_.cursor > 0) {
					TTY_con_.cursor--;
					TTY_con_.buffer[TTY_con_.cursor] = '\0';
					Back();
				}
				return NULL;
			}

			// check if this is a control char
			if ((key) && (key) < ' ') {
				if (key == '\n') {
					// push it in history
					HistAdd(&TTY_con_);
					strncpy(text, TTY_con_.buffer, sizeof(text)-1);
					TTY_con_.buffer[sizeof(text)-1] = 0;
					FieldClear(&TTY_con_);
					key = '\n';
					write(STDOUT_FILENO, &key, 1);
					write(STDOUT_FILENO, TTY_CONSOLE_PROMPT, strlen(TTY_CONSOLE_PROMPT));

					return text;
				}
				if (key == '\t') {
					Hide();
					// auto completition
					Show();
					return NULL;
				}
				avail = read(STDIN_FILENO, &key, 1);
				if (avail != -1) {
					// VT 100 keys
					if (key == '[' || key == 'O') {
						avail = read(STDIN_FILENO, &key, 1);
						if (avail != -1) {
							switch (key) {
								case 'A':
									history = HistPrev();
									if (history) {
										Hide();
										TTY_con_ = *history;
										Show();
									}

									FlushIn();
									return NULL;
									break;

								case 'B':
									history = HistNext();
									Hide();
									if (history)
										TTY_con_ = *history;
									else
										FieldClear(&TTY_con_);

									Show();
									FlushIn();
									return NULL;
									break;

								case 'C':
									return NULL;

								case 'D':
									return NULL;
							}
						}
					}
				}
				FlushIn();
				return NULL;
			}

			if (TTY_con_.cursor >= (int)(sizeof(text) - 1))
				return NULL;

			// push regular character
			TTY_con_.buffer[TTY_con_.cursor] = key;
			TTY_con_.cursor++; // next char will always be '\0'
			// print the current line (this is differential)
			write(STDOUT_FILENO, &key, 1);
		}

		return NULL;
	} else if (stdin_active_) {
		int len;
		fd_set fdset;
		struct timeval timeout;

		FD_ZERO(&fdset);
		FD_SET(STDIN_FILENO, &fdset); // stdin
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if(select (STDIN_FILENO + 1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(STDIN_FILENO, &fdset))
			return NULL;

		len = read(STDIN_FILENO, text, sizeof(text));
		if (len == 0) { // eof!
			stdin_active_ = false;
			return NULL;
		}

		if (len < 1)
			return NULL;
		text[len-1] = 0;    // rip off the /n and terminate

		return text;
	}

	return NULL;
}

void ConsoleTty::Print(const char *msg, ...) {
	if (!msg[0])
		return;

	this->Lock();

	va_list argptr;
	static char	string[32000];
	static int index = 0;

	memset(&string, 0, sizeof(string));
	va_start (argptr, msg);
	vsprintf (string, msg, argptr);
	va_end (argptr);

	Hide();
	AnsiColorPrint(va("^7[^5Q3df ^7]: %s", string));

	if (!ttycon_on_)
		return;

	// Only print prompt when msg ends with a newline, otherwise the console
	//   might get garbled when output does not fit on one line.
	if (msg[strlen(msg) - 1] == '\n') {
		Show();

		// Run CON_Show the number of times it was deferred.
		while (ttycon_show_overdue_ > 0) {
			Show();
			ttycon_show_overdue_--;
		}
	} else // Defer calling CON_Show
		ttycon_show_overdue_++;

	this->Unlock();
}

void ConsoleTty::PrintInfo(const char *msg, ...) {
	if (!msg[0])
		return;
	this->Lock();

	va_list argptr;
	static char	string[32000];
	static int index = 0;

	memset(&string, 0, sizeof(string));
	va_start (argptr, msg);
	vsprintf (string, msg, argptr);
	va_end (argptr);

	Hide();
	AnsiColorPrint(va("^7[^3Info ^7]: %s", string));

	if (!ttycon_on_)
		return;

	// Only print prompt when msg ends with a newline, otherwise the console
	//   might get garbled when output does not fit on one line.
	if (msg[strlen(msg) - 1] == '\n') {
		Show();

		// Run CON_Show the number of times it was deferred.
		while (ttycon_show_overdue_ > 0) {
			Show();
			ttycon_show_overdue_--;
		}
	} else // Defer calling CON_Show
		ttycon_show_overdue_++;

	this->Unlock();
}

void ConsoleTty::PrintError(const char *msg, ...) {
	if (!msg[0])
		return;

	this->Lock();

	va_list argptr;
	static char	string[32000];
	static int index = 0;

	memset(&string, 0, sizeof(string));
	va_start (argptr, msg);
	vsprintf (string, msg, argptr);
	va_end (argptr);

	Hide();
	AnsiColorPrint(va("^7[^1Error^7]: %s", string));

	if (!ttycon_on_)
		return;

	// Only print prompt when msg ends with a newline, otherwise the console
	//   might get garbled when output does not fit on one line.
	if (msg[strlen(msg) - 1] == '\n') {
		Show();

		// Run CON_Show the number of times it was deferred.
		while (ttycon_show_overdue_ > 0) {
			Show();
			ttycon_show_overdue_--;
		}
	} else // Defer calling CON_Show
		ttycon_show_overdue_++;

	this->Unlock();
}

void ConsoleTty::Show() {
	int i;

	if( ttycon_on_ ) {
		ttycon_hide_--;

		if (ttycon_hide_ == 0) {
			write(STDOUT_FILENO, TTY_CONSOLE_PROMPT, strlen(TTY_CONSOLE_PROMPT));
			if (TTY_con_.cursor)
				for (i=0; i<TTY_con_.cursor; i++)
					write(STDOUT_FILENO, TTY_con_.buffer+i, 1);
		}
	}
}

void ConsoleTty::Hide() {
	int i;

	if( ttycon_on_ ) {

		if (ttycon_hide_) {
			ttycon_hide_++;
			return;
		}
		if (TTY_con_.cursor>0)
			for (i=0; i<TTY_con_.cursor; i++)
				Back();

		// Delete prompt
		for (i = strlen(TTY_CONSOLE_PROMPT); i > 0; i--)
			Back();

		ttycon_hide_++;
	}
}

void ConsoleTty::FieldClear(field_t *edit) {
	memset(edit->buffer, 0, MAX_EDIT_LINE);
	edit->cursor = 0;
	edit->scroll = 0;
}

void ConsoleTty::FlushIn() {
	char key;
	while (read(STDIN_FILENO, &key, 1)!=-1);
}

void ConsoleTty::Back() {
	char key;

	key = '\b';
	write(STDOUT_FILENO, &key, 1);
	key = ' ';
	write(STDOUT_FILENO, &key, 1);
	key = '\b';
	write(STDOUT_FILENO, &key, 1);
}

void ConsoleTty::HistAdd(field_t *field) {
	int i;

	// Don't save blank lines in history.
	if (!field->cursor)
		return;

	// make some room
	for (i=CON_HISTORY-1; i>0; i--)
		ttyEditLines_[i] = ttyEditLines_[i-1];

	ttyEditLines_[0] = *field;

	if (hist_count_<CON_HISTORY)
		hist_count_++;

	hist_current_ = -1; // re-init
}

field_t *ConsoleTty::HistPrev() {
	int hist_prev;
	hist_prev = hist_current_ + 1;
	if (hist_prev >= hist_count_)
		return NULL;

	hist_current_++;
	return &(ttyEditLines_[hist_current_]);
}

field_t *ConsoleTty::HistNext() {
	if (hist_current_ >= 0)
		hist_current_--;

	if (hist_current_ == -1)
		return NULL;

	return &(ttyEditLines_[hist_current_]);
}

void ConsoleTty::AnsiColorPrint(const char *msg) {
	static char buffer[MAXPRINTMSG];
	int length = 0;
	static int  q3ToAnsi[8] =
	{
		30, // COLOR_BLACK
		31, // COLOR_RED
		32, // COLOR_GREEN
		33, // COLOR_YELLOW
		34, // COLOR_BLUE
		36, // COLOR_CYAN
		35, // COLOR_MAGENTA
		0   // COLOR_WHITE
	};

	while(*msg) {
		if(Q_IsColorString(msg) || *msg == '\n') {
			// First empty the buffer
			if(length > 0) {
				buffer[ length ] = '\0';
				fputs( buffer, stderr );
				length = 0;
			}

			if( *msg == '\n' ) {
				// Issue a reset and then the newline
				fputs( "\033[0m\n", stderr );
				msg++;
			} else {
				// Print the color code
				snprintf(buffer, sizeof(buffer), "\033[%dm", q3ToAnsi[ColorIndex(*(msg + 1))]);
				fputs( buffer, stderr );
				msg += 2;
			}
		} else {
			if(length >= MAXPRINTMSG - 1)
				break;

			buffer[length] = *msg;
			length++;
			msg++;
		}
	}

	// Empty anything still left in the buffer
	if(length > 0) {
		buffer[length] = '\0';
		fputs(buffer, stderr);
	}
}
