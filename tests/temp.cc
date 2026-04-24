int main() {
  int i = 0;
  for (int j = 0; j <= 10; j++) {
    i++;
  }

  int k = 0;
  while (k < 5) {
    k++;
  }

  int l = 0;
  do {
    l++;
  } while (l < 5);

  int option = 2;
  switch (option) {
    case 1:
      i++;
      break;
    case 2:
      k++;
      break;
    default:
      l++;
      break;
  }

  return 0;
}
