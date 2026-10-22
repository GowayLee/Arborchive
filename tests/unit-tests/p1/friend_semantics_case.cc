class Host;
class Secret;
using SecretAlias = Secret;
void declared_friend(Host &);
void overloaded(int);
void overloaded(double);
void unrelated();
struct MemberTarget { static void access(Host &); };

class Host {
  friend void declared_friend(Host &);  // P1:declared
  friend void introduced_friend(Host &);  // P1:introduced
  friend void introduced_friend(Host &);  // P1:repeated
  friend int inline_friend(Host &) { return 7; }  // P1:inline
  friend class Secret;  // P1:class
  friend SecretAlias;  // P1:alias
  friend struct Introduced;  // P1:introduced_class
  friend void MemberTarget::access(Host &);  // P1:member
  friend void overloaded(int);  // P1:overload_int
  friend void overloaded(double);  // P1:overload_double
  friend int;  // P1:ignored_builtin
};

class OtherHost {
  friend class Secret;  // P1:other_owner
  friend void declared_friend(Host &);  // P1:shared_function
};

struct Outer {
  struct Inner {
    friend class Secret;  // P1:nested_owner
  };
};

template <typename T> struct DependentOwner {
  friend class Secret;  // P1:concrete_in_template
  friend T;  // P1:dependent_target
};

class Secret {};
struct Introduced {};
void declared_friend(Host &) {}
void introduced_friend(Host &) {}
void MemberTarget::access(Host &) {}
void overloaded(int) {}
void overloaded(double) {}
void unrelated() {}
