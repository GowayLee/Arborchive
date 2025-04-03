#include "util/id_generator.h"

std::mutex IDGenerator::mutex_;
std::unordered_map<std::string, std::atomic<uint64_t>> IDGenerator::table_ids_;
