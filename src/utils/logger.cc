#include "util/logger/logger.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>

thread_local Logger::LogContext Logger::log_context_;
thread_local bool Logger::is_logging_ = false;

Logger::Logger() : log_level_threshold_(LogLevel::INFO), batch_size_(100) {}

Logger::~Logger() { shutdown(); }

Logger &Logger::getInstance() {
  static Logger instance;
  return instance;
}

bool Logger::init(const LoggerConfig &config) {
  try {
    // Initialize the logger with the given configuration
    if ((is_to_file_ =
             !config.file.empty())) { // Open the log file if specified
      log_file_.open(config.file, std::ios::app);
      if (!log_file_.is_open())
        throw std::runtime_error("Cannot open log file");
    }
    log_level_threshold_ = StringToLevel(config.level);
    batch_size_ = config.batch_size;
    is_to_console_ = config.is_to_console;

    // Start the worker thread to process log messages
    running_ = true;
    worker_thread_ = std::thread(&Logger::processMessages, this);
  } catch (const std::exception &e) {
    std::cerr << "Logger initialization failed: " << e.what() << std::endl;
    return false;
  }
  return true;
}

void Logger::shutdown() {
  if (running_) {
    running_ = false;
    if (worker_thread_.joinable()) {
      worker_thread_.join();
    }
    flushBuffer();
    log_file_.close();
  }
}

void Logger::log(LogMessage &&msg) {
  if (msg.level >= log_level_threshold_) {
    queue_.push(std::move(msg));
  }
}

void Logger::setLogLevel(LogLevel level) { log_level_threshold_ = level; }

void Logger::processMessages() {
  while (running_ || !queue_.empty()) {
    auto msg = queue_.pop();
    batch_buffer_.push_back(std::move(msg));

    if (batch_buffer_.size() >= batch_size_ ||
        (!running_ && !batch_buffer_.empty())) {
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

    ss << "[" << time_str << "] "
       << "[" << levelToString(msg.level) << "] " << msg.file << ":" << msg.line
       << " - " << msg.message;
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

const LogLevel Logger::StringToLevel(const std::string &level) {
  if (level == "DEBUG" || level == "debug") {
    return LogLevel::DEBUG;
  } else if (level == "INFO" || level == "info") {
    return LogLevel::INFO;
  } else if (level == "WARNING" || level == "warning") {
    return LogLevel::WARNING;
  } else if (level == "ERROR" || level == "error") {
    return LogLevel::ERROR;
  } else {
    throw std::runtime_error("Invalid log level");
  }
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