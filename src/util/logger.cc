#include "util/logger/logger.h"
#include "util/logger/macros.h"
#include <chrono>
#include <iostream>
#include <sstream>

thread_local Logger::LogContext Logger::log_context_;
thread_local bool Logger::is_logging_ = false;

Logger &Logger::getInstance() {
  static Logger instance;
  return instance;
}

// Initialize the logger
void Logger::init() {
  is_to_file_ = false;
  is_to_console_ = true;
#ifdef _DEBUG_
  log_level_threshold_ = LogLevel::DEBUG;
  batch_size_ = 5;
#else
  log_level_threshold_ = LogLevel::INFO;
  batch_size_ = 60;
#endif
  running_ = true;
  worker_thread_ = std::thread(&Logger::processMessages, this);
}

// Load the logger configuration
bool Logger::loadConfig(const LoggerConfig &config) {
  try {
    if ((is_to_file_ =
             !config.file.empty())) { // Open the log file if specified
      log_file_.open(config.file, std::ios::app);
      if (!log_file_.is_open())
        throw std::runtime_error("Cannot open log file");
    }
    log_level_threshold_ = StringToLevel(config.level);
    batch_size_ = config.batch_size;
    is_to_console_ = config.is_to_console;
  } catch (const std::exception &e) {
    LOG_ERROR << "Logger config initialization failed: " << e.what()
              << std::endl;
    return false;
  }
  LOG_INFO << "Logger config initialized successfully\n"
           << INDENT_LEFT
           << "Log level: " << levelToString(log_level_threshold_) << "\n"
           << INDENT_LEFT << "Batch size: " << batch_size_ << "\n"
           << INDENT_LEFT << "Log to console: " << (is_to_console_ ? "Yes" : "No") << "\n"
           << INDENT_LEFT << "Log file: " << (is_to_file_ ? config.file : "N/A")
           << std::endl;
  return true;
}

void Logger::stop() {
  if (running_) {
    LOG_INFO << "Stopping logger worker thread" << std::endl;
    running_ = false;
    queue_.stop(); // 通知队列终止

    if (worker_thread_.joinable())
      worker_thread_.join();

    // 处理队列中剩余的日志
    while (!queue_.empty()) {
      auto msg = queue_.pop();
      batch_buffer_.push_back(std::move(msg));
      if (batch_buffer_.size() >= batch_size_) {
        formatOutput(batch_buffer_);
        batch_buffer_.clear();
      }
    }

    // 处理 batch_buffer_ 中剩余的日志
    if (!batch_buffer_.empty()) {
      formatOutput(batch_buffer_);
      batch_buffer_.clear();
    }

    if (log_file_.is_open())
      log_file_.close();
  } else
    LOG_WARNING << "Try to stop already stopped logger worker thread" << std::endl;
}

void Logger::log(LogMessage &&msg) {
  if (msg.level >= log_level_threshold_) {
    queue_.push(std::move(msg));
  }
}

void Logger::setLogLevel(LogLevel level) { log_level_threshold_ = level; }

void Logger::processMessages() {
  try {
    while (running_ || !queue_.empty()) {
      auto msg = queue_.pop();
      batch_buffer_.push_back(std::move(msg));

      if (batch_buffer_.size() >= batch_size_ ||
          (!running_ && !queue_.empty())) {
        formatOutput(batch_buffer_);
        batch_buffer_.clear();
      }
    }

    // 处理 batch_buffer_ 中剩余的日志
    if (!batch_buffer_.empty()) {
      formatOutput(batch_buffer_);
      batch_buffer_.clear();
    }
  } catch (const std::runtime_error &e) {
    // 处理队列终止异常
    if (!batch_buffer_.empty()) {
      formatOutput(batch_buffer_);
      batch_buffer_.clear();
    }
  }
}

void Logger::formatOutput(const std::vector<LogMessage> &messages) {
  std::stringstream ss;
  for (const auto &msg : messages) {
    auto t = std::chrono::system_clock::to_time_t(msg.timestamp);
    char time_str[9];
    // std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
    //               std::localtime(&t));
    std::strftime(time_str, sizeof(time_str), "%H:%M:%S", std::localtime(&t));

#ifdef _DEBUG_
    ss << "[" << time_str << "] "
       << "[" << levelToString(msg.level) << "] " << msg.file << ":" << msg.line
       << " - " << msg.message;
#else
    ss << "[" << time_str << "] "
       << "[" << levelToString(msg.level) << "] " << msg.message;
#endif
  }

  if (is_to_console_)
    std::cout << ss.str(); // 输出到控制台
  if (is_to_file_)
    log_file_ << ss.str() << std::flush; // 输出到文件
}

void Logger::flushBuffer() {
  if (!batch_buffer_.empty()) {
    formatOutput(batch_buffer_);
    batch_buffer_.clear();
  }
}

const char *Logger::levelToString(LogLevel level) {
  switch (level) {
  case LogLevel::DEBUG:
    return "DEBUG";
  case LogLevel::INFO:
    return "INFO";
  case LogLevel::WARNING:
    return "WARNING";
  case LogLevel::ERROR:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}

LogLevel Logger::StringToLevel(const std::string &level) {
  auto it = log_level_map.find(level);
  if (it != log_level_map.end())
    return it->second;
  else
    throw std::runtime_error("Invalid log level");
}

// Logger类的成员函数，用于设置日志级别、文件名和行号
Logger &Logger::operator()(LogLevel level, const char *file, int line) {
  if (level >= log_level_threshold_) {
    is_logging_ = true;
    // 初始化日志上下文，包括日志流、日志级别、文件名和行号
    log_context_ = LogContext{std::ostringstream(), level, file, line};
  } else
    is_logging_ = false;
  return *this; // 返回当前Logger对象，以便支持链式调用
}

// Logger类的成员函数，用于处理流操作符<<
Logger &Logger::operator<<(std::ostream &(*manip)(std::ostream &)) {
  if (is_logging_) {
    manip(log_context_.stream);
    // 检查操作符是否为std::endl
    if (manip == static_cast<std::ostream &(*)(std::ostream &)>(std::endl)) {
      // 如果是，则将日志消息写入日志系统
      log(LogMessage(log_context_.level, log_context_.stream.str(),
                     log_context_.file, log_context_.line));
      is_logging_ = false;
    }
  }
  return *this;
}
