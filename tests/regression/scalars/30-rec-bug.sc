int t (int n) {
  int x = n;
  if (n > 0) t (n - 1);
  return x;
}

void main () {
  int x = t(1);
  assert (x == 1);
}
