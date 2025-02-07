#ifndef _THREAD_SAFE_QUEUE_H_
#define _THREAD_SAFE_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>


template <typename T> class ThreadSafeQueue {
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cond_;

public:
  void push(T item) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(item));
    }
    cond_.notify_one();
  }

  T pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return !queue_.empty(); });
    T item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }
};

#endif // _THREAD_SAFE_QUEUE_H_