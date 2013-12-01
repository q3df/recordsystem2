#ifndef CLIENT_LOGGER_H_
#define CLIENT_LOGGER_H_

#include <string>
#include "Recordsystem.h"

using namespace std;

enum LogType {
	LOG_INFO = 1,
	LOG_TRACE = 2,
	LOG_WARNING = 4,
	LOG_ERROR = 8,
	LOG_DEBUG = 16
};

class Logger {
public:
	~Logger();
	static Logger *GetInstance();

	Logger *Log(LogType type, const char *file, int line, const char *fmt, ...);

	Logger *operator <<(std::string const& value) {
		gRecordsystem->GetSyscalls()->Print(va("%s", value.c_str()));
		return this;
	}

	Logger *operator <<(char const& value) {
		gRecordsystem->GetSyscalls()->Print(va("%s", value));
		return this;
	}

	Logger *operator <<(int value) {
		gRecordsystem->GetSyscalls()->Print(va("%i", value));
		return this;
	}

	Logger *operator <<(float value) {
		gRecordsystem->GetSyscalls()->Print(va("%f", value));
		return this;
	}

private:
	Logger(std::string path);
};

#define LOG(type, fmt, ...) Logger::GetInstance()->Log(type, __FILE__, __LINE__, fmt, __VA_ARGS__)

#endif // CLIENT_LOGGER_H_
