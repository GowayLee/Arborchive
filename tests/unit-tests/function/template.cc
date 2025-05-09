#include <string>

// 测试类模板推导指引的简单示例
template <typename T> struct MyContainer {
  T value;

  MyContainer(T v) : value(v) {}
};

// 显式推导指引 - 从相同类型构造
MyContainer(const char *) -> MyContainer<std::string>;

// 测试函数
void testDeductionGuides() {
  // 测试1: 使用构造函数参数推导
  MyContainer c1(42);   // 推导为 MyContainer<int>
  MyContainer c2(3.14); // 推导为 MyContainer<double>

  // 测试2: 使用显式推导指引
  MyContainer c3("hello"); // 使用推导指引 -> MyContainer<std::string>

  // 测试3: 显式指定模板参数
  MyContainer<std::string> c4("world"); // 不使用推导指引

  // 测试4: 拷贝初始化
  auto c5 = MyContainer(42); // 推导为 MyContainer<int>

  // 测试5: 从相同类型推导
  MyContainer c6 = c1; // 推导为 MyContainer<int>
}

// 更复杂的例子 - 带多个模板参数的类
template <typename T, typename U> struct Pair {
  T first;
  U second;

  Pair(T f, U s) : first(f), second(s) {}
};

// 推导指引 - 从可转换类型推导
Pair(const char *, const char *) -> Pair<std::string, std::string>;

// 测试函数
void testPairDeduction() {
  // 测试1: 直接推导
  Pair p1(1, 2.0); // 推导为 Pair<int, double>

  // 测试2: 使用推导指引
  Pair p2("a", "b"); // 推导为 Pair<std::string, std::string>

  // 测试3: 混合类型
  Pair p3(1, "b"); // 推导为 Pair<int, const char*>
}

// 主函数
int main() {
  testDeductionGuides();
  testPairDeduction();
  return 0;
}
