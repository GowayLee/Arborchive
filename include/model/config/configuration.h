#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <memory>
#include <string>
#include <vector>

// general配置
struct GeneralConfig {
  std::string source_path;
  std::string output_path;
};

// 编译相关配置
struct CompilationConfig {
  std::vector<std::string> include_paths;
  std::vector<std::string> defines;
  std::string cxx_standard;
  std::vector<std::string> flags;
};

// 数据库相关配置
struct DatabaseConfig {
  std::string path;
  int batch_size;
  int cache_size_mb;
  std::string journal_mode;
  std::string synchronous;
};

// 日志相关配置
struct LoggerConfig {
  std::string level;
  std::string file;
  bool is_to_console;
  int batch_size;
  bool enable_perf_logging;
};

// 完整的配置信息
struct Configuration {
  GeneralConfig general;
  CompilationConfig compilation;
  DatabaseConfig database;
  LoggerConfig logger;
};

#endif