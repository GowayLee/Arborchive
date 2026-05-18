// 定义一个简单的结构体
struct Point {
  int x;
  int y;

  void print() const;
};

// 函数声明
int add(int a, int b);
void printInt(int value);
void printPoint(const Point *p);

class FriendSecret;

class FriendBox {
  friend void inspectFriendBox(FriendBox &);
  friend class FriendSecret;
  int value;
};

void inspectFriendBox(FriendBox &) {}

template <typename T> T identity(T value) { return value; }

template <typename T> struct Holder {
  T value;
};

template <typename T, int N> struct FixedBuffer {
  T value;
};

template <> struct FixedBuffer<int, 4> {
  int value;
};

template <int N> int add_n(int value) { return value + N; }

void use_templates() {
  Holder<int> h{identity(1)};
  Holder<double> hd{identity(2.0)};
}

void use_nontype_template_args() {
  FixedBuffer<int, 4> buffer{};
  int v = add_n<3>(10);
  (void)buffer;
  (void)v;
}

void implicit_casts() {
  int i = 1;
  double d = i;
  int *pi = &i;
  const int *p = pi;
  const void *vp = p;
}

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
  Point *p2 = &p1;
  printPoint(p2);

  return 0;
}

// 函数定义：加法
int add(int a, int b) { return a + b; }

// 函数定义：打印整数
void printInt(int value) {
  // 模拟输出
}

// 函数定义：打印 Point 结构体
void printPoint(const Point *p) {
  if (p) {
    p->print();
  }
}
