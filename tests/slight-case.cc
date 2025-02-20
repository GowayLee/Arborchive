class foo {
public:
  foo() {}
  void bar() {}

  void baz() {}
};

int add(int a, int b) { return a + b; }

int main() {
  int x = 10;
  int y = 20;
  int z = add(x, y);
  return z;
}
