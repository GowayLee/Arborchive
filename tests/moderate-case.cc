// 定义一个简单的结构体
struct Point {
    int x;
    int y;

    void print() const;
};

// 函数声明
int add(int a, int b);
void printInt(int value);
void printPoint(const Point* p);

void Point::print() const {
    // 打印结构体成员
    printInt(x);
    printInt(y);
}

// 主函数
int main() {
    // 变量声明
    int a = 5;
    int b = 10;
    int sum = add(a, b);

    // 控制流：if 语句
    if (sum > 10) {
        printInt(sum);
    } else {
        printInt(0);
    }

    // 控制流：for 循环
    for (int i = 0; i < 3; ++i) {
        printInt(i);
    }

    // 结构体使用
    Point p1 = {1, 2};
    p1.print();

    // 指针使用
    Point* p2 = &p1;
    printPoint(p2);

    return 0;
}

// 函数定义：加法
int add(int a, int b) {
    return a + b;
}

// 函数定义：打印整数
void printInt(int value) {
    // 模拟输出
}

// 函数定义：打印 Point 结构体
void printPoint(const Point* p) {
    if (p) {
        p->print();
    }
}
