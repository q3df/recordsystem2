#include "Logger.h"
#include <cstdarg>
#include <string.h>
#include <stdio.h>


Logger::Logger(std::string path) {
}

Logger::~Logger() {
}

Logger *Logger::GetInstance() {
	static Logger *instance;

	if(!instance)
		instance = new Logger(string("TEST.log"));

	return instance;
}

Logger *Logger::Log(LogType type, const char *file, int line, const char *fmt, ...) {
	va_list argptr;
	char input[4096];
	memset(&input, 0, sizeof(input));

	va_start(argptr, fmt);
	vsnprintf(input, sizeof(input), fmt, argptr);
	va_end(argptr);

	switch(type) {
	case LOG_INFO:
		RS_Print(va("%s:%i|INFO: %s\n", file, line, input));
		break;
	case LOG_TRACE:
		RS_Print(va("%s:%i|TRACE: %s\n", file, line, input));
		break;
	case LOG_WARNING:
		RS_Print(va("%s:%i|WARNING: %s\n", file, line, input));
		break;
	case LOG_ERROR:
		RS_Print(va("%s:%i|ERROR: %s\n", file, line, input));
		break;
	case LOG_DEBUG:
		RS_Print(va("%s:%i|DEBUG: %s\n", file, line, input));
		break;
	}

	return this;
}
