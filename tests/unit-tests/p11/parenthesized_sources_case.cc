struct Box { int value; };
int target(int input) { return input; }
using Callback = int (*)(int);

double scalar_plain(int value) { return static_cast<double>(value); } // CHECK: scalar_plain
double scalar_paren(int value) { return static_cast<double>((value)); } // CHECK: scalar_paren
double scalar_nested(int value) { return static_cast<double>(((value))); } // CHECK: scalar_nested
double member_plain(Box &box) { return static_cast<double>(box.value); } // CHECK: member_plain
double member_paren(Box &box) { return static_cast<double>((box.value)); } // CHECK: member_paren
double member_nested(Box &box) { return static_cast<double>(((box.value))); } // CHECK: member_nested
double arrow_plain(Box *box) { return static_cast<double>(box->value); } // CHECK: arrow_plain
double arrow_paren(Box *box) { return static_cast<double>(((box->value))); } // CHECK: arrow_paren
Callback routine_plain() { return target; } // CHECK: routine_plain
Callback routine_paren() { return (target); } // CHECK: routine_paren
Callback routine_nested() { return ((target)); } // CHECK: routine_nested
Callback address_plain() { return &target; } // CHECK: address_plain
Callback address_outer() { return (&target); } // CHECK: address_outer
Callback address_inner() { return &((target)); } // CHECK: address_inner

struct MethodBox { int method(); int (*callback)(int); };
int method_dot_plain(MethodBox &box) { return box.method(); } // CHECK: method_dot_plain
int method_dot_paren(MethodBox &box) { return (box.method)(); } // CHECK: method_dot_paren
int method_dot_nested(MethodBox &box) { return ((box.method))(); } // CHECK: method_dot_nested
int method_arrow_plain(MethodBox *box) { return box->method(); } // CHECK: method_arrow_plain
int method_arrow_paren(MethodBox *box) { return (box->method)(); } // CHECK: method_arrow_paren
int method_arrow_nested(MethodBox *box) { return ((box->method))(); } // CHECK: method_arrow_nested
Callback callback_dot_plain(MethodBox &box) { return box.callback; } // CHECK: callback_dot_plain
Callback callback_dot_nested(MethodBox &box) { return ((box.callback)); } // CHECK: callback_dot_nested
Callback callback_arrow_nested(MethodBox *box) { return ((box->callback)); } // CHECK: callback_arrow_nested
