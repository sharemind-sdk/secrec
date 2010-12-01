int glob = -1;

int fib (int n) {
  assert (n >= 0);
  if (n < 2) return 1;
  return (fib(n-1) + fib(n-2));
}

void main() {
  int old = 1;
  int cur = 1;
  int n = 1;
  while (n < 10) {
    assert (cur == fib (n));
    int t = old + cur;
    old = cur;
    cur = t;
    glob = cur;
    n += 1;
  }
}
