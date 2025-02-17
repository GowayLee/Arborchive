#include "interface/cli.h"
#include "util/logger/logger_macros.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

void printBanner();

void Cli::init(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i)
    args.push_back(argv[i]);

  arg_definitions = {
      {"-c", "--config-path", "Specify config.toml path (required)", true,
       [this](const std::string &value) { options.config_path = value; }},

      {"-s", "--source", "Specify source file or directory path (required)",
       true, [this](const std::string &value) { options.source_path = value; }},

      {"-o", "--output", "Specify output database file path (required)", true,
       [this](const std::string &value) { options.output_path = value; }},

      {"-q", "--quiet", "Suppress logger's output to console", false,
       [this](const std::string &) { options.quiet = true; }},

      {"-h", "--help", "Show help information", false,
       [this](const std::string &) { options.show_help = true; }},

      {"-v", "--version", "Show version information", false,
       [this](const std::string &) { options.show_version = true; }}};
}

void Cli::parseArgs() {
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];

    auto it =
        std::find_if(arg_definitions.begin(), arg_definitions.end(),
                     [&arg](const CLArgDef &def) {
                       return arg == def.short_name || arg == def.long_name;
                     });

    if (it != arg_definitions.end()) {
      if (it->requires_value) {
        if (i + 1 >= args.size()) // check if next argument exists
          throw std::runtime_error("Missing value for argument: " + arg);
        it->handler(args[++i]);
        LOG_DEBUG << "Argument: " << arg << " Value: " << args[i] << std::endl;
      } else
        it->handler("");
    } else
      throw std::runtime_error("Unknown argument: " + arg);
  }
}

void Cli::showHelp() const {
  std::cout << "Usage: arborchive [options]\n\n";
  std::cout << "Options:\n";

  for (const auto &arg : arg_definitions) {
    std::cout << "  " << std::left << std::setw(4) << arg.short_name
              << std::left << std::setw(20) << arg.long_name << arg.description
              << '\n';
  }
}

void Cli::showVersion() const {
  std::cout << "Arborchive version " << VERSION << std::endl;
}

int Cli::process() {
  try {
    parseArgs();

    if (options.show_help) {
      showHelp();
      return 0;
    }

    if (options.show_version) {
      showVersion();
      return 0;
    }

    if (!options.isValid()) { // check if all required arguments are provided
      std::cerr << "Error: Missing required arguments\n";
      showHelp();
      return 1;
    }

    // output file ./banner to console
#ifndef _DEBUG_
    printBanner();
#endif

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    showHelp();
    return 1;
  }

  return 2;
}

void printBanner() {
    std::cout << "         /\\                .\n";
    std::cout << "     _  / |               /                               /       .-.\n";
    std::cout << "    (  /  |  .  ).--.    /-.    .-._.   ).--.  .-.       /-.      `-'  )   .-.   .-.\n";
    std::cout << "     `/.__|_.' /        /   )  (   )   /      (         /   |    /    (   /    ./.-'_\n";
    std::cout << " .:' /    |   /       .'`--'`-  `-'   /        `---' _.'    | _.(__.   \\_/     (__.'\n";
    std::cout << "(__.'     `-' \n\n";
}
