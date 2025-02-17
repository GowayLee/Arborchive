#ifndef _LOGGER_MACROS_H
#define _LOGGER_MACROS_H

#include "logger.h"
#include "model/log/log_message.h"

#define LOG(level) Logger::getInstance()(level, __FILE__, __LINE__)

#define LOG_DEBUG LOG(LogLevel::DEBUG)
#define LOG_INFO LOG(LogLevel::INFO)
#define LOG_WARNING LOG(LogLevel::WARNING)
#define LOG_ERROR LOG(LogLevel::ERROR)

#define INDENT_LEFT "                  "

/* Example:
 * LOG_INFO << "This is a test" << std::endl;
 * LOG_DEBUG << "Args: " << arg1 << ", " << arg2 << std::endl;
 * LOG_ERROR << "Error in parsing file" << std::endl;
*/

#endif // _LOGGER_MACROS_H