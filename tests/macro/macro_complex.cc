#define STR(x) #x
#define XSTR(x) STR(x)
#define CAT(a, b) a##b
#define ADD(a, b) ((a) + (b))
#define WRAP_ADD(x, y) ADD((x), (y))
#define TWICE_WRAP(v) WRAP_ADD(v, WRAP_ADD(v, 1))
#define INNER(x) (x)
#define MIDDLE(x) INNER(x)
#define OUTER(x) MIDDLE(x)

int main() {
  int xy = 7;
  int pasted = CAT(x, y);

  const char *raw = STR(WRAP_ADD(1, 2));
  const char *expanded = XSTR(WRAP_ADD(1, 2));
  const char *chain_text = XSTR(OUTER(7));

  int nested_chain = OUTER(42);
  int nested = TWICE_WRAP(3);
  return pasted + nested + nested_chain + (raw[0] == 'W') +
         (expanded[0] == '(') + (chain_text[0] == '(');
}
