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
