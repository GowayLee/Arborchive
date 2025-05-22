class foo {
public:
  foo() {}
  void bar() {}

  void baz() {}
};

int add(const int a, const int b) { return a + b; }

int main() {
  int x = 10;
  int y = 20;
  int z = add(x, y);
  return z;
}
