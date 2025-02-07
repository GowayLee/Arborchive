#ifndef _LOG_MESSAGE_H_
#define _LOG_MESSAGE_H_

#include "enum/log_level.h"
#include <chrono>
#include <string>

struct LogMessage {
  LogLevel level;
  std::chrono::system_clock::time_point timestamp;
  std::string message;
  std::string file;
  int line;

  LogMessage(LogLevel lvl, std::string msg, std::string src_file, int src_line)
      : level(lvl), timestamp(std::chrono::system_clock::now()),
        message(std::move(msg)), file(std::move(src_file)), line(src_line) {}
};

#endif // _LOG_MESSAGE_H_