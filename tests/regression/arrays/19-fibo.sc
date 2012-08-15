
uint [[1]] fibo (uint n) {
  assert (n > 0);
  uint [[1]] out (n) = 1;
  if (n == 1 || n == 2) return out;
  out[1:n] = fibo (n - 1);
  out[2:n] += fibo (n - 2);
  return out;
}

void main () {
  uint [[1]] fibs = fibo (5 :: uint);
  uint c = 1;
  uint n = 1;
  for (uint i = 0; i < 5; ++ i) {
    assert (fibs[i] == c);
    uint t = c + n;
    c = n;
    n = t;
  }
}
