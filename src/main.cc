#include "interface/clang_initializer.h"
#include "interface/cli.h"
#include "interface/config_loader.h"
#include "model/config/cl_args.h"
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    auto &cli = Cli::getInstance();
    cli.init(argc, argv);

    int ret = cli.process();
    if (ret != 2)
      return ret;

    const CLArgs &clargs = cli.getOptions();

    auto &configLoader = ConfigLoader::getInstance();
    configLoader.loadFromCli(clargs); // 从命令行参数加载配置
    if (!configLoader.loadFromFile(clargs.config_path)) {
      std::cerr << "Failed to load config file" << std::endl;
      return 1;
    } else
      std::cout << "Config file: " << clargs.config_path
                << " loaded successfully" << std::endl;

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