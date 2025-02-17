#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "enum/log_level.h"
#include "model/config/configuration.h"
#include "model/log/log_message.h"
#include "util/thread_safe_queue.h"
#include <atomic>
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

  // 定义一个结构体 LogContext，用于存储日志相关的上下文信息
  struct LogContext {
    std::ostringstream stream; // 使用 std::ostringstream
                               // 来存储日志信息，可以方便地进行字符串拼接
    LogLevel level;
    const char *file;
    int line;
  };

  // 定义一个静态的 thread_local 变量 log_context_，用于存储当前线程的日志上下文
  // thread_local 修饰符表示该变量在每个线程中都有独立的副本，不会在线程间共享
  static thread_local LogContext log_context_;
  static thread_local bool is_logging_; // 标记当前线程是否正在进行日志记录

  Logger() = default;
  ~Logger() = default;

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  void processMessages();
  void formatOutput(const std::vector<LogMessage> &messages);
  void flushBuffer();

public:
  static Logger &getInstance();

  void init();
  bool loadConfig(const LoggerConfig &config);
  void stop();
  void log(LogMessage &&msg);
  void setLogLevel(LogLevel level);
  const char *levelToString(LogLevel level);
  LogLevel StringToLevel(const std::string &level);

  // 定义一个函数调用运算符，用于设置日志级别和代码文件信息
  Logger &operator()(LogLevel level, const char *file, int line);

  template <typename T> Logger &operator<<(const T &value) {
    if (is_logging_)
      log_context_.stream << value;
    return *this;
  }

  Logger &operator<<(std::ostream &(*manip)(std::ostream &));
};

#endif // _LOGGER_H_
