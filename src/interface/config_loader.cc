#include "interface/config_loader.h"
#include "model/config/cl_args.h"

#include <iostream>
#include <toml.hpp>

bool ConfigLoader::loadFromFile(const std::string &config_file) {
  try {
    auto data = toml::parse(config_file);

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
    config.logging.level = toml::find<std::string>(logging, "level");
    config.logging.file = toml::find<std::string>(logging, "file");
    config.logging.enable_perf_logging =
        toml::find<bool>(logging, "enable_perf_logging");

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error loading config file: " << e.what() << std::endl;
    return false;
  }
}

void ConfigLoader::loadFromCli(const CLArgs &args) {
  config.config_path = args.config_path;
  config.source_path = args.source_path;
  config.output_path = args.output_path;
}