// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// some parts copied from ioquake3

#include "Console.h"


#ifdef WIN32
#	include "ConsoleWin32.h"
#else
#	include "ConsoleTty.h"
#endif

Console *gConsole;

vec4_t	g_color_table[8] = {
	{0.0, 0.0, 0.0, 1.0},
	{1.0, 0.0, 0.0, 1.0},
	{0.0, 1.0, 0.0, 1.0},
	{1.0, 1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0, 1.0},
	{0.0, 1.0, 1.0, 1.0},
	{1.0, 0.0, 1.0, 1.0},
	{1.0, 1.0, 1.0, 1.0},
};


Console::Console() {
	this->mtx_ = PTHREAD_MUTEX_INITIALIZER;
}


Console::~Console() {
	pthread_mutex_destroy(&this->mtx_);
}


void Console::Init() {
	#ifdef WIN32
		gConsole = new ConsoleWin32();
	#else
		gConsole = new ConsoleTty();
	#endif
}


void Console::Dispose() {
	delete gConsole;
	gConsole = NULL;
}


void Console::Lock() {
	pthread_mutex_lock(&this->mtx_);
}


void Console::Unlock() {
	pthread_mutex_unlock(&this->mtx_);	
}


const char *Console::va(const char *msg, ...) {
	va_list	argptr;
	static char	string[2][32000];	// in case va is called by nested functions
	static int index = 0;
	char	*buf;

	buf = string[index & 1];
	index++;
	
	va_start (argptr, msg);
	vsprintf (buf, msg, argptr);
	va_end (argptr);

	return buf;
}