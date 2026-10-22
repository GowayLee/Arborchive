// Only the instantiated inner aggregate subtree is covered here. The generic
// explicit-brace root has a separate, pre-existing source-key limitation.
template<class T> void template_identity() {
#line 100 "generated_same.cc"
  struct Same { T x; }; // FIELD:x
  using Outer = Same;
  {
#line 100 "generated_same.cc"
  struct Same { Outer g; }; // FIELD:g
    struct Root { Same s; };
    Root value{555}; // INIT
  }
}
template void template_identity<int>();
