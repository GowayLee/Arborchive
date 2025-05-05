#include <iostream>
#include <string>
#include <vector>

// 普通类
class Foo {
public:
    Foo() {}
    void bar() {}

    // 嵌套类
    class Nested {
    public:
        void nestedMethod() {}
    };
};

// 模板类
template<typename T, typename U>
class TemplateClass {
public:
    T value;
    void templateMethod(U param) {}
};

// 模板特化
template<>
class TemplateClass<int, float> {
public:
    void specializedMethod() {}
};

// 命名空间测试
namespace MyNamespace {
    class NamespacedClass {
    public:
        void namespacedMethod() {}
    };

    // 匿名结构体
    struct {
        int anonymousField;
    } anonymousInstance;
}

// 类型别名
using IntVector = std::vector<int>;
using TemplateAlias = TemplateClass<double, char>;

// 模板函数
template<typename T>
T templateFunction(T a, T b) {
    return a + b;
}

// 普通函数
int add(int a, int b) { return a + b; }

int main() {
    // 基本类型测试
    int x = 10;
    int y = 20;
    int z = add(x, y);

    // 实例化测试
    Foo f;
    f.bar();

    Foo::Nested n;
    n.nestedMethod();

    TemplateClass<std::string, bool> tc;
    tc.templateMethod(true);

    TemplateClass<int, float> tcs;
    tcs.specializedMethod();

    MyNamespace::NamespacedClass nc;
    nc.namespacedMethod();

    // 使用类型别名
    IntVector iv;
    TemplateAlias ta;

    // 调用模板函数
    double d = templateFunction(3.14, 2.71);

    return z;
}

