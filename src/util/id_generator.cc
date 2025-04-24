#include "util/id_generator.h"

std::mutex IDGenerator::mutex_;
std::unordered_map<std::type_index, std::atomic<int>> IDGenerator::type_ids_;
