struct ConcreteBase {
  int concrete_value;
};

template <typename T>
struct DependentTypeBase : T {
  int dependent_value;
};

template <typename T>
struct TemplateBase {
  T templated_value;
};

template <typename T>
struct DependentTemplateBase : TemplateBase<T> {
  T dependent_template_value;
};

struct DiamondRoot {
  int root_value;
};

struct DiamondLeft : virtual DiamondRoot {
  int left_value;
};

struct DiamondRight : virtual DiamondRoot {
  int right_value;
};

struct DiamondFinal : DiamondLeft, DiamondRight {
  int final_value;
};

struct EmptyBase {};

struct EmptyDerived : EmptyBase {
  int value;
};

struct NoUniqueAddressHolder {
  [[no_unique_address]] EmptyBase empty_member;
  int value;
};

struct AnonymousUnionHolder {
  union {
    int as_int;
    char as_char;
  };
  int after;
};

struct AnonymousStructHolder {
  struct {
    int nested;
  };
  int after;
};

struct PackedBits {
  unsigned first : 3;
  unsigned : 0;
  unsigned second : 9;
  unsigned third : 7;
};

int main() {
  DependentTypeBase<ConcreteBase> dependent_type{};
  DependentTemplateBase<int> dependent_template{};
  DiamondFinal diamond{};
  EmptyDerived empty_derived{};
  NoUniqueAddressHolder no_unique{};
  AnonymousUnionHolder anonymous_union{};
  AnonymousStructHolder anonymous_struct{};
  PackedBits bits{};

  anonymous_union.as_int = 1;
  anonymous_struct.nested = 2;

  return dependent_type.dependent_value +
         dependent_template.dependent_template_value + diamond.final_value +
         empty_derived.value + no_unique.value + anonymous_union.as_int +
         anonymous_struct.nested + bits.first + bits.second + bits.third;
}
