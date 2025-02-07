#ifndef _LOGGER_MACROS_H
#define _LOGGER_MACROS_H

#include "logger.h"
#include "model/log/log_message.h"

#define LOG(level, message)                                                    \
  Logger::getInstance().log(LogMessage(level, message, __FILE__, __LINE__))

#define LOG_DEBUG(message) LOG(LogLevel::DEBUG, message)
#define LOG_INFO(message) LOG(LogLevel::INFO, message)
#define LOG_WARNING(message) LOG(LogLevel::WARNING, message)
#define LOG_ERROR(message) LOG(LogLevel::ERROR, message)

#endif // _LOGGER_MACROS_H