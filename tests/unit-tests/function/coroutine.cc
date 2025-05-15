#include <coroutine>

// 简单的协程返回对象
struct Generator {
  struct promise_type {
    int current_value;

    Generator get_return_object() {
      return Generator{
          std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() { std::terminate(); }
    std::suspend_always yield_value(int value) {
      current_value = value;
      return {};
    }
    void return_void() {}
  };

  std::coroutine_handle<promise_type> handle;

  explicit Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
  ~Generator() {
    if (handle)
      handle.destroy();
  }

  int next() {
    handle.resume();
    return handle.promise().current_value;
  }
};

// 协程函数
Generator sequence() {
  for (int i = 0; i < 3; ++i) {
    co_yield i;
  }
}

int main() {
  Generator gen = sequence();
  while (true) {
    int value = gen.next();
    if (value == 2)
      break;
  }
  return 0;
}
