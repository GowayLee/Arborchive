#ifndef _CONFIG_LOADER_H_
#define _CONFIG_LOADER_H_

#include <string>

class ConfigLoader {
public:
    bool load(const std::string& filename);
    // Add getters for configuration parameters

private:
    // Add members to store configuration
};

#endif // _CONFIG_LOADER_H_