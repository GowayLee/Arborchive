#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "enum/log_level.h"
#include "model/config/configuration.h"
#include "model/log/log_message.h"
#include "util/thread_safe_queue.h"
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>


class Logger {
private:
  ThreadSafeQueue<LogMessage> queue_;
  std::ofstream log_file_;
  std::atomic<bool> running_{false};
  std::thread worker_thread_;
  LogLevel log_level_threshold_;
  std::vector<LogMessage> batch_buffer_;
  size_t batch_size_;
  bool is_to_file_;
  bool is_to_console_;

  Logger();
  ~Logger();

  void processMessages();
  void formatOutput(const std::vector<LogMessage> &messages);
  void flushBuffer();

public:
  static Logger &getInstance();

  bool init(const LoggerConfig &config);
  void shutdown();
  void log(LogMessage &&msg);
  void setLogLevel(LogLevel level);
  const char *levelToString(LogLevel level);
  const LogLevel StringToLevel(const std::string &level);
};

#endif // _LOGGER_H_