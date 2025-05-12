// 使用typedef定义函数指针类型
typedef int (*MathFunc)(int, int);
// 使用using定义函数类型（C++11）
using MathFunc2 = int(int, int);

int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

MathFunc funcPtr = &add;
