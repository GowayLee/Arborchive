#include "core/router.h"
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
    auto &logger = Logger::getInstance();
    logger.init();

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
    configLoader.mergeFromCli(clargs);

    if (!logger.loadConfig(configLoader.getConfig().logger))
      return 1;

    auto &clangIndexer = ClangIndexer::getInstance();
    if (!clangIndexer.loadConfig(configLoader.getConfig()))
      return 1;

    auto &dbManager = DatabaseManager::getInstance();
    if (!dbManager.loadConfig(configLoader.getConfig().database)) {
      LOG_ERROR << "Failed to load database config" << std::endl;
      return 1;
    }
    dbManager.start();

    // 创建翻译单元并解析
    Router &router = Router::getInstance();
    // router.processCompilation(configLoader.getConfig());

    dbManager.stop();
    std::cout << "main: stop logger" << std::endl;
    logger.stop();

    std::cout << "Done" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }
}
