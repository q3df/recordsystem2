// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConsoleWin32.h"
#include <Windows.h>
#include <stdio.h>

ConsoleWin32::ConsoleWin32() {
	int i = 0;

	qconsole_linelen_ = 0;
	qconsole_drawinput_ = true;
	qconsole_cursor_pos_offset_ = 0;

	CONSOLE_SCREEN_BUFFER_INFO info;

	// handle Ctrl-C or other console termination
	//SetConsoleCtrlHandler(CtrlHandler, TRUE);

	qconsole_hin_ = GetStdHandle(STD_INPUT_HANDLE);
	if(qconsole_hin_ == INVALID_HANDLE_VALUE)
		return;

	qconsole_hout_ = GetStdHandle(STD_OUTPUT_HANDLE);
	if(qconsole_hout_ == INVALID_HANDLE_VALUE)
		return;

	GetConsoleMode(qconsole_hin_, &qconsole_orig_mode_);

	// allow mouse wheel scrolling
	SetConsoleMode(qconsole_hin_, qconsole_orig_mode_ & ~ENABLE_MOUSE_INPUT);

	FlushConsoleInputBuffer(qconsole_hin_);

	GetConsoleScreenBufferInfo(qconsole_hout_, &info);
	qconsole_attrib_ = info.wAttributes;
	qconsole_backgroundAttrib_ = qconsole_attrib_ & (BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_INTENSITY);

	SetConsoleTitle("recordsystemd - console");

	qconsole_history_pos_ = -1;
	qconsole_history_lines_ = 0;
	qconsole_history_oldest_ = 0;
	
	// initialize history
	for( i = 0; i < QCONSOLE_HISTORY; i++ )
		qconsole_history_[i][0] = '\0';

	// set text color to white
	SetConsoleTextAttribute(qconsole_hout_, ColorCharToAttrib(COLOR_WHITE));

	Show();

	this->PrintInfo("Console initialized...\n");
}


ConsoleWin32::~ConsoleWin32() {
	Hide();
	SetConsoleMode(qconsole_hin_, qconsole_orig_mode_);
	SetConsoleCursorInfo(qconsole_hout_, &qconsole_orig_cursorinfo_);
	SetConsoleTextAttribute(qconsole_hout_, qconsole_attrib_);
	CloseHandle(qconsole_hout_);
	CloseHandle(qconsole_hin_);
}


char *ConsoleWin32::Input() {
	CONSOLE_SCREEN_BUFFER_INFO binfo;
	COORD cursorPos;
	INPUT_RECORD buff[ MAX_EDIT_LINE ];
	DWORD count = 0, events = 0;
	WORD key = 0;
	unsigned int i;
	int newlinepos = -1, z;

	if(!GetNumberOfConsoleInputEvents(qconsole_hin_, &events))
		return NULL;

	if(events < 1)
		return NULL;
  
	// if we have overflowed, start dropping oldest input events
	if(events >= MAX_EDIT_LINE) {
		ReadConsoleInput(qconsole_hin_, buff, 1, &events);
		return NULL;
	} 

	if(!ReadConsoleInput(qconsole_hin_, buff, events, &count))
		return NULL;

	FlushConsoleInputBuffer(qconsole_hin_);

	for(i = 0; i < count; i++) {
		if(buff[i].EventType != KEY_EVENT)
			continue;

		if(!buff[i].Event.KeyEvent.bKeyDown)
			continue;

		key = buff[i].Event.KeyEvent.wVirtualKeyCode;

		if( key == VK_RETURN ) {
			newlinepos = i;
			break;
		} else if( key == VK_UP ) {
			HistPrev();
			break;
		} else if( key == VK_DOWN ) {
			HistNext();
			break;
		} else if( key == VK_TAB ) {
			break;
		} else if( key == VK_LEFT ) {
			GetConsoleScreenBufferInfo(qconsole_hout_, &binfo);
			if((qconsole_linelen_-qconsole_cursor_pos_offset_) > 0)
				qconsole_cursor_pos_offset_++;

			// set curor position
			cursorPos.Y = binfo.dwCursorPosition.Y;
			cursorPos.X = qconsole_linelen_ > binfo.srWindow.Right ? binfo.srWindow.Right : qconsole_linelen_ - qconsole_cursor_pos_offset_;

			SetConsoleCursorPosition(qconsole_hout_, cursorPos);
			break;
		} else if( key == VK_RIGHT ) {
			GetConsoleScreenBufferInfo(qconsole_hout_, &binfo);
			if(qconsole_cursor_pos_offset_ > 0)
				qconsole_cursor_pos_offset_--;

			// set curor position
			cursorPos.Y = binfo.dwCursorPosition.Y;
			cursorPos.X = qconsole_linelen_ > binfo.srWindow.Right ? binfo.srWindow.Right : qconsole_linelen_ - qconsole_cursor_pos_offset_;

			SetConsoleCursorPosition(qconsole_hout_, cursorPos);
			break;
		} else if( key == VK_END ) {
			GetConsoleScreenBufferInfo(qconsole_hout_, &binfo);
			qconsole_cursor_pos_offset_ = 0;

			// set curor position
			cursorPos.Y = binfo.dwCursorPosition.Y;
			cursorPos.X = qconsole_linelen_ > binfo.srWindow.Right ? binfo.srWindow.Right : qconsole_linelen_ - qconsole_cursor_pos_offset_;

			SetConsoleCursorPosition(qconsole_hout_, cursorPos);
		} else if( key == VK_HOME ) {
			GetConsoleScreenBufferInfo(qconsole_hout_, &binfo);
			qconsole_cursor_pos_offset_ = qconsole_linelen_;

			// set curor position
			cursorPos.Y = binfo.dwCursorPosition.Y;
			cursorPos.X = qconsole_linelen_ > binfo.srWindow.Right ? binfo.srWindow.Right : qconsole_linelen_ - qconsole_cursor_pos_offset_;

			SetConsoleCursorPosition(qconsole_hout_, cursorPos);
		}

		if( qconsole_linelen_ < sizeof(qconsole_line_) - 1 ) {
			char c = buff[ i ].Event.KeyEvent.uChar.AsciiChar;
			
			if( key == VK_BACK) {
				int pos = (qconsole_linelen_ > 0) ? qconsole_linelen_ - 1 : 0;

				if(qconsole_cursor_pos_offset_ > 0) {
					if( (qconsole_linelen_ - qconsole_cursor_pos_offset_) > 0) {
						for(z=pos-qconsole_cursor_pos_offset_; z < qconsole_linelen_; z++)
							qconsole_line_[z] = qconsole_line_[z+1];

						qconsole_line_[pos] = '\0';
						qconsole_linelen_ = pos;
					}
				}else{
					qconsole_line_[pos] = '\0';
					qconsole_linelen_ = pos;
				}
			} else if(c) {
				if(qconsole_cursor_pos_offset_ > 0) {
					int pos = qconsole_linelen_ > 0 ? qconsole_linelen_ : 0;

					if( (qconsole_linelen_ - qconsole_cursor_pos_offset_) >= 0) {
						for(z=pos; z >= (pos-qconsole_cursor_pos_offset_); z--)
							qconsole_line_[z+1] = qconsole_line_[z];

						qconsole_line_[pos-qconsole_cursor_pos_offset_] = c;
						qconsole_linelen_++;
						qconsole_line_[qconsole_linelen_] = '\0';
					}
				}else{
					qconsole_line_[qconsole_linelen_++] = c;
					qconsole_line_[qconsole_linelen_] = '\0';
				}
			}
		}
	}

	if( newlinepos < 0) {
		Show();
		return NULL;
	}

	if(!qconsole_linelen_) {
		Show();
		PrintInput( "\n" );
		return NULL;
	}

	qconsole_linelen_ = 0;
	qconsole_cursor_pos_offset_ = 0;
	Show();

	HistAdd();
	PrintInput("%s\n", qconsole_line_);

	return qconsole_line_;
}


void ConsoleWin32::Print(const char *msg, ...) {
	this->Lock();

	va_list argptr;
	static char string[32000];
	int index = 0;

	memset(&string, 0, sizeof(string));
	va_start (argptr, msg);
	vsprintf (string, msg, argptr);
	va_end (argptr);

	this->Hide();

	/*std::string *str = new std::string();
	for(index = 0; index < strlen(string); index++) {
		if(string[index] == '\n' && index == 0 && index < strlen(string)-1) {
			str->append("\n^7[^5Q3df ^7]: ");
		}else if(string[index] == '\n' && index > 0 && index < strlen(string)-1) {
			str->append("\n^7[^5Q3df ^7]: ");
		}else{
			str->append(&string[index], 1);
		}
	}
	this->WindowsColorPrint( this->va("^7[^5Q3df ^7]: %s", str->c_str()) );
	delete str;
	*/

	this->WindowsColorPrint( string );
	this->Show();

	this->Unlock();
}


void ConsoleWin32::PrintInput(const char *msg, ...) {
	this->Lock();
	va_list argptr;
	static char	string[32000];
	static int index = 0;

	memset(&string, 0, sizeof(string));
	va_start (argptr, msg);
	vsprintf (string, msg, argptr);
	va_end (argptr);

	this->Hide();
	this->WindowsColorPrint( this->va("^5>^7 %s", string) );
	this->Show();
	this->Unlock();
}


void ConsoleWin32::PrintInfo(const char *msg, ...) {
	this->Lock();
	va_list argptr;
	static char	string[32000];
	static int index = 0;

	memset(&string, 0, sizeof(string));
	va_start (argptr, msg);
	vsprintf (string, msg, argptr);
	va_end (argptr);

	this->Hide();
	this->WindowsColorPrint( this->va("^7[^3Info ^7]^7: %s", string) );
	this->Show();
	this->Unlock();
}


void ConsoleWin32::PrintError(const char *msg, ...) {
	this->Lock();
	va_list argptr;
	static char	string[32000];
	static int index = 0;

	memset(&string, 0, sizeof(string));
	va_start (argptr, msg);
	vsprintf (string, msg, argptr);
	va_end (argptr);

	this->Hide();
	this->WindowsColorPrint( this->va("^7[^1Error^7]^7: %s", string) );
	this->Show();
	this->Unlock();
}


void ConsoleWin32::Show() {
	CONSOLE_SCREEN_BUFFER_INFO binfo;
	COORD writeSize = {MAX_EDIT_LINE, 1};
	COORD writePos = {0, 0};
	SMALL_RECT writeArea = {0, 0, 0, 0};
	COORD cursorPos;
	int i;
	CHAR_INFO line[MAX_EDIT_LINE];
	WORD attrib;
	int realI = 0;

	GetConsoleScreenBufferInfo(qconsole_hout_, &binfo);

	// if we're in the middle of printf, don't bother writing the buffer
	if(!qconsole_drawinput_)
		return;

	writeArea.Left = 0;
	writeArea.Top = binfo.dwCursorPosition.Y; 
	writeArea.Bottom = binfo.dwCursorPosition.Y; 
	writeArea.Right = MAX_EDIT_LINE;

	// set color to white
	attrib = ColorCharToAttrib(COLOR_WHITE);

	// build a space-padded CHAR_INFO array
	for( i = 0; i < MAX_EDIT_LINE; i++ ) {
		if(this->printPrompt_)
			realI = i-2;
		else
			realI = i;

		if( (i == 0 || i == 1) && this->printPrompt_ ) {
			if(i == 0) {
				const char *tmp = "5";
				attrib = ColorCharToAttrib(*tmp);
				line[i].Char.AsciiChar = '>';
			}else{
				const char *tmp = "7";
				attrib = ColorCharToAttrib(*tmp);
				line[i].Char.AsciiChar = ' ';
			}
		}else if(realI < qconsole_linelen_) {
			if(Q_IsColorString(qconsole_line_ + realI))
				attrib = ColorCharToAttrib( *(qconsole_line_ + realI + 1) );

			line[i].Char.AsciiChar = qconsole_line_[realI];
		} else
			line[i].Char.AsciiChar = ' ';

		line[i].Attributes = attrib;
	}

	if(qconsole_linelen_ > binfo.srWindow.Right)
		WriteConsoleOutput(qconsole_hout_, line + (qconsole_linelen_ - binfo.srWindow.Right), writeSize, writePos, &writeArea);
	else
		WriteConsoleOutput(qconsole_hout_, line, writeSize, writePos, &writeArea);

	// set curor position
	cursorPos.Y = binfo.dwCursorPosition.Y;
	cursorPos.X = qconsole_linelen_ > binfo.srWindow.Right ? binfo.srWindow.Right : qconsole_linelen_-qconsole_cursor_pos_offset_;
	if(this->printPrompt_)
		cursorPos.X += 2;

	SetConsoleCursorPosition(qconsole_hout_, cursorPos);
}


void ConsoleWin32::Hide() {
	int realLen;
	realLen = qconsole_linelen_;
	printPrompt_ = false;
	// remove input line from console output buffer
	qconsole_linelen_ = 0;
	Show();

	this->printPrompt_ = true;
	qconsole_linelen_ = realLen;
}


void ConsoleWin32::WindowsColorPrint(const char *msg) {
	static char buffer[ MAXPRINTMSG ];
	int length = 0;

	while( *msg )
	{
		qconsole_drawinput_ = (*msg == '\n');

		if(Q_IsColorString(msg) || *msg == '\n') {
			// First empty the buffer
			if(length > 0) {
				buffer[length] = '\0';
				fputs(buffer, stderr);
				length = 0;
			}

			if(*msg == '\n') {
				// Reset color and then add the newline
				SetConsoleTextAttribute(qconsole_hout_, ColorCharToAttrib(COLOR_WHITE));
				fputs("\n", stderr);
				msg++;
			} else {
				// Set the color
				SetConsoleTextAttribute(qconsole_hout_, ColorCharToAttrib( *(msg + 1) ));
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


WORD ConsoleWin32::ColorCharToAttrib(char color) {
	WORD attrib;

	if (color == COLOR_WHITE) {
		// use console's foreground and background colors
		attrib = qconsole_attrib_;
	} else {
		float *rgba = g_color_table[ColorIndex(color)];

		// set foreground color
		attrib = ( rgba[0] >= 0.5 ? FOREGROUND_RED		: 0 ) |
				( rgba[1] >= 0.5 ? FOREGROUND_GREEN		: 0 ) |
				( rgba[2] >= 0.5 ? FOREGROUND_BLUE		: 0 ) |
				( rgba[3] >= 0.5 ? FOREGROUND_INTENSITY	: 0 );

		// use console's background color
		attrib |= qconsole_backgroundAttrib_;
	}

	return attrib;
}


BOOL WINAPI ConsoleWin32::CtrlHandler(DWORD sig) {
	this->Print(va("SIGNAL: %i\n", sig));
	return TRUE;
}


void ConsoleWin32::HistAdd() {
	strncpy(qconsole_history_[qconsole_history_oldest_], qconsole_line_, sizeof(qconsole_history_[qconsole_history_oldest_])-1 );
	qconsole_history_[qconsole_history_oldest_][sizeof(qconsole_history_[qconsole_history_oldest_])-1] = 0;

	if( qconsole_history_lines_ < QCONSOLE_HISTORY )
		qconsole_history_lines_++;

	if( qconsole_history_oldest_ >= QCONSOLE_HISTORY - 1 )
		qconsole_history_oldest_ = 0;
	else
		qconsole_history_oldest_++;

	qconsole_history_pos_ = qconsole_history_oldest_;
}


void ConsoleWin32::HistPrev() {
	int pos;

	pos = (qconsole_history_pos_ < 1) ? (QCONSOLE_HISTORY - 1) : (qconsole_history_pos_ - 1);

	// don' t allow looping through history
	if(pos == qconsole_history_oldest_ || pos >= qconsole_history_lines_)
		return;

	qconsole_history_pos_ = pos;
	strncpy(qconsole_line_, qconsole_history_[qconsole_history_pos_], sizeof( qconsole_line_)-1);
	qconsole_line_[sizeof(qconsole_line_)-1] = 0;

	qconsole_linelen_ = strlen(qconsole_line_);
}


void ConsoleWin32::HistNext() {
	int pos;

	// don' t allow looping through history
	if( qconsole_history_pos_ == qconsole_history_oldest_ )
		return;

	pos = (qconsole_history_pos_ >= QCONSOLE_HISTORY - 1) ?	0 : (qconsole_history_pos_ + 1); 

	// clear the edit buffer if they try to advance to a future command
	if(pos == qconsole_history_oldest_) {
		qconsole_history_pos_ = pos;
		qconsole_line_[0] = '\0';
		qconsole_linelen_ = 0;
		return;
	}

	qconsole_history_pos_ = pos;
	strncpy(qconsole_line_, qconsole_history_[qconsole_history_pos_], sizeof(qconsole_line_)-1);
	qconsole_line_[sizeof(qconsole_line_)-1] = 0;
	qconsole_linelen_ = strlen(qconsole_line_);
}
