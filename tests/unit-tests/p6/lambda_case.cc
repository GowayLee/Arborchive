int lambda_case() {
  int x = 1;

  auto a = [] { return 1; };
  auto b = [x] { return x; };
  auto c = [&x] { return x; };
  auto d = [=] { return x; };
  auto e = [&] -> int { return x; };

  return a() + b() + c() + d() + e();
}

int main() { return lambda_case(); }
