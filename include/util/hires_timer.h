#ifndef _HIRES_TIMER_H_
#define _HIRES_TIMER_H_

#include <chrono>

class HighResTimer {
  using Clock = std::chrono::high_resolution_clock;
  Clock::time_point start_;
  std::clock_t cpu_start_;

public:
  void start() {
    start_ = Clock::now();
    cpu_start_ = std::clock();
  }

  double elapsed() const {
    return std::chrono::duration<double>(Clock::now() - start_).count();
  }

  double cpu_time() const {
    return (std::clock() - cpu_start_) / double(CLOCKS_PER_SEC);
  }
};

#endif
