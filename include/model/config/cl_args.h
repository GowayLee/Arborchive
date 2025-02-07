#ifndef _CL_ARGS_H_
#define _CL_ARGS_H_

#include <string>

struct CLArgs {
  std::string config_path;
  std::string source_path;
  std::string output_path;
  bool quiet{false};
  bool show_help{false};
  bool show_version{false};

  bool isValid() const {
    return !config_path.empty() && !source_path.empty() && !output_path.empty();
  }
};

#endif // _CL_ARGS_H_