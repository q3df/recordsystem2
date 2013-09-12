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

void Logger::Log(LogType type, const char *file, int line, const char *fmt, ...) {
	va_list argptr;
	char input[4096];
	memset(&input, 0, sizeof(input));

	va_start(argptr, fmt);
	vsnprintf(input, sizeof(input), fmt, argptr);
	va_end(argptr);

	switch(type) {
	case LOG_DEBUG:
		printf("%s:%i|DEBUG: %s\r\n", file, line, input);
		break;
	case LOG_VERBOSE:
		printf("%s:%i|VERBOSE: %s\r\n", file, line, input);
		break;
	case LOG_INFO:
		printf("%s:%i|INFO: %s\r\n", file, line, input);
		break;
	}
}
