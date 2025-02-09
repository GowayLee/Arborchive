#ifndef _LOGGER_MACROS_H
#define _LOGGER_MACROS_H

#include "logger.h"
#include "model/log/log_message.h"

#define LOG(level) Logger::getInstance()(level, __FILE__, __LINE__)

#define LOG_DEBUG LOG(LogLevel::DEBUG)
#define LOG_INFO LOG(LogLevel::INFO)
#define LOG_WARNING LOG(LogLevel::WARNING)
#define LOG_ERROR LOG(LogLevel::ERROR)

#endif // _LOGGER_MACROS_H