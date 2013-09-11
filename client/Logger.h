#ifndef CLIENT_LOGGER_H_
#define CLIENT_LOGGER_H_

#include <string>

using namespace std;

enum LogType {
	LOG_DEBUG = 1,
	LOG_VERBOSE = 2,
	LOG_INFO = 4
};

class Logger {
public:
	~Logger();
	static Logger *GetInstance();
	void Log(LogType type, const char *file, int line, const char *fmt, ...);

private:
	Logger(std::string path);
};

#define LOG(x1, x2, ...) Logger::GetInstance()->Log(x1, __FILE__, __LINE__, x2, __VA_ARGS__)

#endif // CLIENT_LOGGER_H_
