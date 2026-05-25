struct PublicBase {
  int public_base_value;
};

struct ProtectedBase {
  int protected_base_value;
};

struct PrivateBase {
  int private_base_value;
};

struct VirtualBase {
  int virtual_base_value;
};

struct SingleDerived : public PublicBase {
  int single_value;
};

struct MultipleDerived : public PublicBase, protected ProtectedBase,
                         private PrivateBase {
  int multiple_value;
};

struct VirtualDerived : public virtual VirtualBase {
  int virtual_value;
};

class ClassDefaultPrivate : PublicBase {
  int class_value;
};

int main() {
  SingleDerived single{};
  MultipleDerived multiple{};
  VirtualDerived virtual_derived{};
  ClassDefaultPrivate default_private{};

  return single.single_value + multiple.multiple_value +
         virtual_derived.virtual_value + sizeof(default_private);
}
