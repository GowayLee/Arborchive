#ifndef _LOG_LEVEL_H_
#define _LOG_LEVEL_H_

#include <map>
#include <string>

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

static const std::map<std::string, LogLevel> log_level_map = {
    {"DEBUG", LogLevel::DEBUG},     {"INFO", LogLevel::INFO},
    {"WARNING", LogLevel::WARNING}, {"ERROR", LogLevel::ERROR},
    {"debug", LogLevel::DEBUG},     {"info", LogLevel::INFO},
    {"warning", LogLevel::WARNING}, {"error", LogLevel::ERROR}};

#endif