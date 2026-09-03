#include "p13_support.h"

/// Returns its argument unchanged.
int p13_documented(int value);

int p13_documented(int value) { return value; }

/**
 * Adds one to the supplied value.
 *
 * The block markers are removed from persisted contents.
 */
int p13_block_documented(int value) { return value + 1; }

// Ordinary comments are outside the P13 safe subset.
int p13_ordinary_comment(int value) { return value - 1; }

/// Variable documentation remains deferred.
int p13_documented_variable = 0;

#define P13_DECLARE_FUNCTION(name) int name() { return 13; }
/// Documentation attached to a macro-generated declaration remains deferred.
P13_DECLARE_FUNCTION(p13_macro_documented)

struct P13CommentedMember {
  //! Returns the stable member value.
  int value() const { return 13; }
};

int main() {
  P13CommentedMember member;
  return p13_documented(member.value()) + p13_block_documented(0) +
         p13_ordinary_comment(1) + p13_macro_documented() +
         p13_documented_variable - p13_support_value;
}
