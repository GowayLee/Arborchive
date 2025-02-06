#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <iostream>

enum LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
public:
    static void Log(LogLevel level, const std::string& message);
    
    // 示例：快速日志方法
    static void Debug(const std::string& msg) { Log(DEBUG, msg); }
    static void Error(const std::string& msg) { Log(ERROR, msg); }
};

#endif // _LOGGER_H_