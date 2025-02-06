#ifndef _CLI_H_
#define _CLI_H_

#include <functional>
#include <map>
#include <optional>
#include <string>

class CliEntryPoint {
public:
  // 参数配置结构体
  struct ProgramOptions {
    std::string compile_commands_path;
    std::string source_path;
    std::string output_path;
    bool show_help{false};
    bool show_version{false};

    bool isValid() const {
      return !compile_commands_path.empty() && !source_path.empty() &&
             !output_path.empty();
    }
  };

  explicit CliEntryPoint(int argc, char *argv[]);

  // 运行CLI程序
  int run();

  // 获取解析后的配置
  const ProgramOptions &getOptions() const { return options; }

private:
  std::vector<std::string> args;
  ProgramOptions options;

  // 参数定义结构
  struct ArgDefinition {
    std::string short_name;
    std::string long_name;
    std::string description;
    bool requires_value;
    std::function<void(const std::string &)> handler;
  };

  std::vector<ArgDefinition> arg_definitions;

  void initializeArgDefinitions();
  void parseArguments();
  void showHelp() const;
  void showVersion() const;
  static constexpr const char *VERSION = "1.0.0";
};

#endif // _CLI_H_