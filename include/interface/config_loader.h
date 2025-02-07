#ifndef _CONFIG_LOADER_H_
#define _CONFIG_LOADER_H_

#include "model/config/cl_args.h"
#include "model/config/configuration.h"
#include <memory>
#include <string>
#include <vector>

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

  /*
   * 从命令行参数中加载配置信息, 如果命令行参数中存在与配置文件中相同的配置项,
   * 则覆盖配置文件中的配置项
   */
  void mergeFromCli(const CLArgs &args);

  // 获取配置信息
  const Configuration &getConfig() const { return config; }

private:
  Configuration config;

  ConfigLoader() = default;
  ~ConfigLoader() = default;
};

#endif // _CONFIG_LOADER_H_