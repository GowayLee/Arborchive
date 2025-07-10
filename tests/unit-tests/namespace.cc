// Test file for namespace parsing functionality

// Basic namespace
namespace basic_namespace {
int variable = 42;
void function() {}

class MyClass {
public:
  int member;
};
} // namespace basic_namespace

// Nested namespaces
namespace outer {
namespace inner {
int nested_var = 10;
void nested_func() {}
} // namespace inner

int outer_var = 20;
} // namespace outer

// Inline namespace
inline namespace inline_ns {
int inline_var = 100;
void inline_func() {}
} // namespace inline_ns

// Anonymous namespace
namespace {
int anonymous_var = 30;
void anonymous_func() {}
} // namespace

// Namespace with various members
namespace comprehensive {
// Variables
int global_var = 1;
const int const_var = 2;
static int static_var = 3;

// Functions
void simple_func() {}
int return_func() { return 42; }
template <typename T> void template_func(T value) {}

// Classes
class SimpleClass {};

struct SimpleStruct {
  int member;
};

// Enums
enum Color { RED, GREEN, BLUE };
enum class Status { OK, ERROR };

// Nested namespace
namespace nested {
int nested_value = 999;
}

// Type aliases
using IntPtr = int *;
typedef int Integer;
} // namespace comprehensive

// Multiple namespace blocks with same name
namespace multi_block {
int first_var = 1;
}

namespace multi_block {
int second_var = 2;
}

// Namespace with using declarations
namespace using_test {

void test_using() {}
} // namespace using_test


int main() {
  basic_namespace::variable = 50;
  outer::inner::nested_var = 15;
  inline_var = 200; // inline namespace allows direct access

  comprehensive::SimpleClass obj;
  comprehensive::Color color = comprehensive::RED;

  return 0;
}