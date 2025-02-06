#include "interface/cli.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

CliEntryPoint::CliEntryPoint(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  initializeArgDefinitions();
}

void CliEntryPoint::initializeArgDefinitions() {
  arg_definitions = {
      {"-c", "--compile-commands", "Specify compile_commands.json path", true,
       [this](const std::string &value) {
         options.compile_commands_path = value;
       }},

      {"-s", "--source", "Specify source file or directory path", true,
       [this](const std::string &value) { options.source_path = value; }},

      {"-o", "--output", "Specify output database file path", true,
       [this](const std::string &value) { options.output_path = value; }},

      {"-h", "--help", "Show help information", false,
       [this](const std::string &) { options.show_help = true; }},

      {"-v", "--version", "Show version information", false,
       [this](const std::string &) { options.show_version = true; }}};
}

void CliEntryPoint::parseArguments() {
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];

    auto it =
        std::find_if(arg_definitions.begin(), arg_definitions.end(),
                     [&arg](const ArgDefinition &def) {
                       return arg == def.short_name || arg == def.long_name;
                     });

    if (it != arg_definitions.end()) {
      if (it->requires_value) {
        if (i + 1 >= args.size()) {
          throw std::runtime_error("Missing value for argument: " + arg);
        }
        it->handler(args[++i]);
      } else {
        it->handler("");
      }
    } else {
      throw std::runtime_error("Unknown argument: " + arg);
    }
  }
}

void CliEntryPoint::showHelp() const {
  std::cout << "Usage: ast_parser [options]\n\n";
  std::cout << "Options:\n";

  for (const auto &arg : arg_definitions) {
    std::cout << "  " << std::left << std::setw(4) << arg.short_name
              << std::left << std::setw(20) << arg.long_name << arg.description
              << '\n';
  }
}

void CliEntryPoint::showVersion() const {
  std::cout << "AST Parser version " << VERSION << std::endl;
}

int CliEntryPoint::run() {
  try {
    parseArguments();

    if (options.show_help) {
      showHelp();
      return 0;
    }

    if (options.show_version) {
      showVersion();
      return 0;
    }

    if (!options.isValid()) {
      std::cerr << "Error: Missing required arguments\n";
      showHelp();
      return 1;
    }

    // TODO: 继续处理，调用ConfigLoader等
    return 0;

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}