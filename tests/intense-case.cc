#include <iostream>
#include <string>
#include <vector>

// 定义一个简单的结构体
struct Person {
  std::string name;
  int age;

  void printInfo() const {
    std::cout << "Name: " << name << ", Age: " << age << std::endl;
  }
};

// 函数声明
int add(int a, int b);
void printVector(const std::vector<int> &vec);

// 主函数
int main() {
  // 变量声明
  int x = 10;
  int y = 20;
  int sum = add(x, y);

  // 控制流：if 语句
  if (sum > 25) {
    std::cout << "Sum is greater than 25" << std::endl;
  } else {
    std::cout << "Sum is less than or equal to 25" << std::endl;
  }

  // 控制流：for 循环
  std::vector<int> numbers = {1, 2, 3, 4, 5};
  printVector(numbers);

  // 结构体使用
  Person person;
  person.name = "Alice";
  person.age = 30;
  person.printInfo();

  return 0;
}

// 函数定义：加法
int add(int a, int b) { return a + b; }

// 函数定义：打印 vector
void printVector(const std::vector<int> &vec) {
  for (int num : vec) {
    std::cout << num << " ";
  }
  std::cout << std::endl;
}
