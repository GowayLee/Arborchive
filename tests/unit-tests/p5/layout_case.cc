struct LayoutBase {
  int base_value;
};

struct SecondaryBase {
  char secondary_value;
};

struct LayoutDerived : public LayoutBase, public SecondaryBase {
  char tag;
  int value;
  unsigned flags : 3;
  unsigned mode : 5;
};

struct LayoutVirtualBase {
  int virtual_base_value;
};

struct LayoutVirtualDerived : public virtual LayoutVirtualBase {
  char marker;
  int payload;
};

int main() {
  LayoutDerived derived{};
  LayoutVirtualDerived virtual_derived{};

  return derived.value + virtual_derived.payload;
}
