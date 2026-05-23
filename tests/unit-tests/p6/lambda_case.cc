int lambda_case() {
  int x = 1;

  auto a = [] { return 1; };
  auto b = [x] { return x; };
  auto c = [&x] { return x; };
  auto d = [=] { return x; };
  auto e = [&] -> int { return x; };

  return a() + b() + c() + d() + e();
}

struct ThisCaptureCase {
  int value;

  int run() {
    auto explicit_this = [this] { return value; };
    auto implicit_this_by_ref = [&] { return value; };
    auto implicit_this_by_copy_default = [=] { return value; };

    return explicit_this() + implicit_this_by_ref() +
           implicit_this_by_copy_default();
  }
};

struct StarThisCaptureCase {
  int value;

  int run() {
    auto star_this = [*this] { return value; };
    return star_this();
  }
};

int init_capture_case() {
  int x = 1;
  auto by_value_init = [y = x + 1] { return y; };
  auto by_ref_init = [&r = x] { return r; };

  return by_value_init() + by_ref_init();
}

unsigned long vla_capture_case(int n) {
  int arr[n];
  auto f = [&] { return sizeof(arr); };

  return f();
}

template <typename... Ts> int pack_capture_case(Ts... xs) {
  auto f = [...ys = xs] { return sizeof...(ys); };
  return static_cast<int>(f());
}

struct StructuredPair {
  int first;
  int second;
};

void structured_binding_capture_case() {
  auto [a, b] = StructuredPair{1, 2};
  auto f = [a] { return a; };

  (void)b;
  (void)f();
}

int main() {
  ThisCaptureCase this_case{1};
  StarThisCaptureCase star_this_case{2};

  structured_binding_capture_case();

  return lambda_case() + this_case.run() + star_this_case.run() +
         init_capture_case() + static_cast<int>(vla_capture_case(3)) +
         pack_capture_case(1, 2);
}
