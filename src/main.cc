#include "interface/clang_initializer.h"
#include "interface/cli.h"
#include "interface/config_loader.h"
#include "model/config/cl_args.h"
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    auto &cli = Cli::getInstance();
    cli.init(argc, argv);

    if (!cli.process())
      return 1;

    const CLArgs &clargs = cli.getOptions();

    auto &configLoader = ConfigLoader::getInstance();
    if (!configLoader.loadFromFile(clargs.config_path)) {
      std::cerr << "Failed to load compilation database" << std::endl;
      return 1;
    }
    configLoader.loadFromCli(clargs); // 从命令行参数加载配置

    // 初始化Logger

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