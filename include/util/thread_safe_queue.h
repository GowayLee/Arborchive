#ifndef _THREAD_SAFE_QUEUE_H_
#define _THREAD_SAFE_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>

template <typename T> class ThreadSafeQueue {
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cond_;
  bool stopped_ = false; // 新增终止标志

public:
  void push(T item) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stopped_)
        throw std::runtime_error("Queue is stopped");
      queue_.push(std::move(item));
    }
    cond_.notify_one();
  }

  T pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return !queue_.empty() || stopped_; });
    if (stopped_ && queue_.empty())
      throw std::runtime_error("Queue is stopped");
    T item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

  void stop() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopped_ = true;
    }
    cond_.notify_all(); // 唤醒所有等待线程
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  bool isStopped() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stopped_;
  }

  void resume() {
    std::lock_guard<std::mutex> lock(mutex_);
    stopped_ = false;
    cond_.notify_all();
  }
};

#endif // _THREAD_SAFE_QUEUE_H_