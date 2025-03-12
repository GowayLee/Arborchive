#include "interface/config_loader.h"
#include "model/config/cl_args.h"
#include "util/logger/macros.h"
#include <iostream>
#include <toml.hpp>

bool ConfigLoader::loadFromFile(const std::string &config_file) {
  try {
    auto data = toml::parse(config_file);

    // 解析general部分
    auto &general = toml::find(data, "general");
    config.general.source_path =
        toml::find<std::string>(general, "source_path");
    config.general.output_path =
        toml::find<std::string>(general, "output_path");

    // 解析compilation部分
    auto &compilation = toml::find(data, "compilation");
    config.compilation.include_paths =
        toml::find<std::vector<std::string>>(compilation, "include_paths");
    config.compilation.defines =
        toml::find<std::vector<std::string>>(compilation, "defines");
    config.compilation.cxx_standard =
        toml::find<std::string>(compilation, "cxx_standard");
    config.compilation.flags =
        toml::find<std::vector<std::string>>(compilation, "flags");

    // 解析database部分
    auto &database = toml::find(data, "database");
    config.database.path = toml::find<std::string>(database, "path");
    config.database.batch_size = toml::find<int>(database, "batch_size");
    config.database.cache_size_mb = toml::find<int>(database, "cache_size_mb");
    config.database.journal_mode =
        toml::find<std::string>(database, "journal_mode");
    config.database.synchronous =
        toml::find<std::string>(database, "synchronous");

    // 解析logging部分
    auto &logging = toml::find(data, "logging");
    config.logger.level = toml::find<std::string>(logging, "level");
    config.logger.file = toml::find<std::string>(logging, "file");
    config.logger.is_to_console = toml::find<bool>(logging, "is_to_console");
    config.logger.batch_size = toml::find<int>(logging, "batch_size");
    config.logger.enable_perf_logging =
        toml::find<bool>(logging, "enable_perf_logging");

    return true;
  } catch (const std::exception &e) {
    LOG_ERROR << "Error loading config file: " << e.what() << std::endl;
    return false;
  }
}

/*
 * 从命令行参数中加载配置信息, 如果命令行参数中存在与配置文件中相同的配置项,
 * 则覆盖配置文件中的配置项
 */

void ConfigLoader::mergeFromCli(const CLArgs &args) {
  config.general.source_path = args.source_path;
  LOG_INFO << "merged source_path: " << config.general.source_path << std::endl;
  config.general.output_path = args.output_path;
  LOG_DEBUG << "merged output_path: " << config.general.output_path
            << std::endl;
  config.logger.is_to_console = !args.quiet;
  LOG_DEBUG << "merged is_to_console: " << config.logger.is_to_console
            << std::endl;
  config.compilation.working_directory = args.working_directory;
  LOG_DEBUG << "merged working_directory: " << config.compilation.working_directory
            << std::endl;
}
