
int [[1]] fibo (int n) {
  assert (n > 0);
  int [[1]] out [n] = 1;
  if (n == 1 || n == 2) return out;
  out[1:n] = fibo (n - 1);
  out[2:n] += fibo (n - 2);
  return out;
}

void main () {
  int [[1]] fibs = fibo (5);
  int i;
  int c = 1;
  int n = 1;
  for (i = 0; i < 5; i += 1) {
    assert (fibs[i] == c);
    int t = c + n;
    c = n;
    n = t;
  }
}
