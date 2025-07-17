#include "util/id_generator.h"

std::mutex IDGenerator::mutex_;
std::atomic<int> IDGenerator::global_id_ = 0;
