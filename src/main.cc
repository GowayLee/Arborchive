#include "interface/clang_initializer.h"
#include "interface/cli.h"
#include "interface/config_loader.h"
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    CliEntryPoint cli(argc, argv);
    int result = cli.run();
    if (result != 0) {
      return result;
    }

    // 获取解析后的配置
    const auto &options = cli.getOptions();

    // 初始化配置加载器
    ConfigLoader configLoader;
    if (!configLoader.load(options.compile_commands_path)) {
      std::cerr << "Failed to load compilation database" << std::endl;
      return 1;
    }

    // 初始化libclang
    LibclangInitializer libclangInit;
    if (!libclangInit.initialize()) {
      std::cerr << "Failed to initialize libclang" << std::endl;
      return 1;
    }

    // TODO: 继续处理流程

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }
}