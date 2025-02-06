#ifndef _CONFIG_LOADER_H_
#define _CONFIG_LOADER_H_

#include "model/config/cl_args.h"
#include <memory>
#include <string>
#include <vector>

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
struct LoggingConfig {
  std::string level;
  std::string file;
  bool enable_perf_logging;
};

// 完整的配置信息
struct Configuration {
  CompilationConfig compilation;
  DatabaseConfig database;
  LoggingConfig logging;

  // CLI参数(从cli获取)
  std::string config_path;
  std::string source_path;
  std::string output_path;
};

class ConfigLoader {
public:
  ConfigLoader(const ConfigLoader &) = delete;
  ConfigLoader &operator=(const ConfigLoader &) = delete;

  static ConfigLoader &getInstance() {
    static ConfigLoader instance;
    return instance;
  }

  // 加载配置文件
  bool loadFromFile(const std::string &config_file);

  // 加载CLI参数
  void loadFromCli(const CLArgs &args);

  // 获取配置信息
  const Configuration &getConfig() const { return config; }

private:
  Configuration config;

  ConfigLoader() = default;
  ~ConfigLoader() = default;
};

#endif // _CONFIG_LOADER_H_