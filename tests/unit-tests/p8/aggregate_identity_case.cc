struct P8Identity { int x; }; // FIELD:global_x
using P8GlobalIdentity = P8Identity;
void identity_global_local() {
  struct P8Identity { P8GlobalIdentity g; }; // FIELD:shadow_g
  struct Root { P8Identity s; }; // FIELD:shadow_s
  Root shadow{111}; // CASE:shadow
}

void identity_nested_block() {
  struct Same { int x; }; // FIELD:block_x
  using Outer = Same;
  {
    struct Same { Outer g; }; // FIELD:block_g
    struct Root { Same s; }; // FIELD:block_s
    Root block{222}; // CASE:block
  }
}

void identity_record_array() {
  struct P8Identity { P8GlobalIdentity g[1]; }; // FIELD:array_g
  struct Root { P8Identity s[1]; }; // FIELD:array_s
  Root arrays{333}; // CASE:arrays
}

struct P8Forward;
using P8ForwardAlias = P8Forward;
struct P8Forward { int x; }; // FIELD:forward_x
void identity_alias_forward() {
  struct P8Forward { P8ForwardAlias g; }; // FIELD:forward_g
  struct Root { P8Forward s; }; // FIELD:forward_s
  Root forward{444}; // CASE:forward
}
