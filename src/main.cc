#include "db/manager.h"
#include "interface/clang_indexer.h"
#include "interface/cli.h"
#include "interface/config_loader.h"
#include "model/config/cl_args.h"
#include "util/logger/logger.h"
#include "util/logger/macros.h"
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    Logger::getInstance().init(); // 初始化Logger

    auto &cli = Cli::getInstance();
    cli.init(argc, argv);

    int ret = cli.process();
    if (ret != 2)
      return ret;

    const CLArgs &clargs = cli.getOptions();

    auto &configLoader = ConfigLoader::getInstance();
    if (!configLoader.loadFromFile(clargs.config_path)) {
      LOG_ERROR << "Failed to load config file: " << clargs.config_path
                << std::endl;
      return 1;
    } else
      LOG_INFO << "Config file: " << clargs.config_path
               << " loaded successfully" << std::endl;
    configLoader.mergeFromCli(clargs); // 从命令行参数加载配置, 覆盖默认配置

    // 加载日志配置
    if (!Logger::getInstance().loadConfig(configLoader.getConfig().logger))
      return 1;

    // 初始化libclang
    auto &clangIndexer = ClangIndexer::getInstance();
    clangIndexer.loadConfig(configLoader.getConfig());
    if (!clangIndexer.init()) {
      LOG_ERROR << "Failed to initialize libclang" << std::endl;
      return 1;
    }

    // 初始化数据库连接
    DatabaseManager::getInstance(configLoader.getConfig().database).start();

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }
}
