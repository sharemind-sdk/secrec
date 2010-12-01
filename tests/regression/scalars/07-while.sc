
int fact (int n) {
  int out = 1;
  while (n > 0) {
    out = out * n;
    n = n - 1;
  }

  return out;
}

void main () {
  assert (fact(0) == 1);
  assert (fact(5) == 120);
}
