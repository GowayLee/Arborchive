#ifndef _CLI_H_
#define _CLI_H_

#include "model/config/cl_args.h"
#include <functional>
#include <map>
#include <optional>
#include <string>


class Cli {
public:
  Cli(const Cli &) = delete;
  Cli &operator=(const Cli &) = delete;

  static Cli &getInstance() {
    static Cli instance;
    return instance;
  }

  void init(int argc, char *argv[]);
  bool process();

  // 获取解析后的配置
  const CLArgs &getOptions() const { return options; }

private:
  std::vector<std::string> args;
  CLArgs options;

  Cli() = default;
  ~Cli() = default;

  // 参数定义结构
  struct CLArgDef {
    std::string short_name;
    std::string long_name;
    std::string description;
    bool requires_value;
    std::function<void(const std::string &)> handler;
  };

  std::vector<CLArgDef> arg_definitions;

  void parseArgs();
  void showHelp() const;
  void showVersion() const;
  static constexpr const char *VERSION = "1.0.0";
};

#endif // _CLI_H_