#include <stdexcept>

// 1. 空异常规范 (throw())
void func_empty_throw() throw();

// 2. 动态异常规范 (throw(type1, type2))
void func_dynamic_throw() throw(std::runtime_error, std::logic_error);

// 3. 单类型异常规范
void func_single_throw() throw(std::bad_alloc);

// 4. 模板类型异常规范
template<typename T>
void func_template_throw() throw(T);

// 5. 无异常规范
void func_no_throw();

// 6. noexcept规范 (C++11+)
void func_noexcept() noexcept;

// 实际定义（测试用）
void func_empty_throw() throw() {}
void func_dynamic_throw() throw(std::runtime_error, std::logic_error) {}
void func_single_throw() throw(std::bad_alloc) {}
template<typename T>
void func_template_throw() throw(T) {}
void func_no_throw() {}
void func_noexcept() noexcept {}

// 测试类
class TestClass {
public:
    // 7. 成员函数的异常规范
    void member_func() throw(std::exception);
};

void TestClass::member_func() throw(std::exception) {}

// 8. 带命名空间的函数
namespace MyNS {
    void namespaced_func() throw(std::range_error);
}

void MyNS::namespaced_func() throw(std::range_error) {}

// 主函数（仅用于编译测试）
int main() {
    func_empty_throw();
    func_dynamic_throw();
    func_single_throw();
    func_template_throw<int>();
    func_no_throw();
    func_noexcept();

    TestClass t;
    t.member_func();

    MyNS::namespaced_func();

    return 0;
}
