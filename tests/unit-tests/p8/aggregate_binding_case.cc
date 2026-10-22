struct P8Base { int base_value; };
struct P8Derived : P8Base { int own_value; };
P8Derived derived{{11}, 22};

struct P8Left { int left_value; };
struct P8Right { int right_value; };
struct P8Multi : P8Left, P8Right { int multi_value; };
P8Multi multi{{31}, {32}, 33};

union P8Choice { int first; int second; };
P8Choice union_first{41};
P8Choice union_second{.second = 42};

struct P8Pair { int x; int y; };
struct P8Nested { P8Pair pair; int tail; };
P8Nested nested{{51, 52}, 53};
P8Nested elided{61, 62, 63};
P8Pair ordinary{71, 72};

struct P8Bits { unsigned : 3; int before; unsigned : 0; int after; };
P8Bits bits{81, 82};

P8Derived elided_base{91, 92};

struct P8Layer { P8Base base; };
struct P8Chain { P8Layer layer; };
P8Chain deep{101};
