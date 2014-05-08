#ifndef SERVER_CONSOLEWIN32_H_
#define SERVER_CONSOLEWIN32_H_

#include <Windows.h>
#include <cstdarg>
#include "Console.h"

extern "C" {
	const char *va( const char *format, ... );
}

class ConsoleWin32 : public Console {
public:
	ConsoleWin32();
	~ConsoleWin32();

	virtual char *Input();
	virtual void Print(const char *msg, ...);
	virtual void PrintInfo(const char *msg, ...);
	virtual void PrintError(const char *msg, ...);
	
protected:
	virtual void PrintInput(const char *msg, ...);
	virtual void Show();
	virtual void Hide();

private:
	virtual WORD ColorCharToAttrib(char color);
	virtual BOOL WINAPI CtrlHandler(DWORD sig);
	virtual void WindowsColorPrint(const char *msg);

	void HistAdd();
	void HistPrev();
	void HistNext();


	WORD qconsole_attrib_;
	WORD qconsole_backgroundAttrib_;

	// saved console status
	DWORD qconsole_orig_mode_;
	CONSOLE_CURSOR_INFO qconsole_orig_cursorinfo_;

	// current edit buffer
	char qconsole_line_[ MAX_EDIT_LINE ];
	int qconsole_linelen_;
	bool qconsole_drawinput_;
	int qconsole_cursor_pos_offset_;
	bool printPrompt_;

	HANDLE qconsole_hout_;
	HANDLE qconsole_hin_;

	// cmd history
	char qconsole_history_[ QCONSOLE_HISTORY ][ MAX_EDIT_LINE ];
	int qconsole_history_pos_;
	int qconsole_history_lines_;
	int qconsole_history_oldest_;
};

#endif // SERVER_CONSOLEWIN32_H_
