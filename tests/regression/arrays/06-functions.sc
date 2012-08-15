
bool all (bool [[1]] arr) {
  for (uint i = 0; i < size(arr); ++ i) {
    if (!arr[i]) return false;
  }

  return true;
}

int [[1]] constant () {
  int [[1]] out (42) = 42;
  return out;
}

int [[1]] double (int [[1]] arr) {
  return arr + arr;
}

void order (int one, int [[1]] two, int three) {
  assert (one == 1);
  assert (all(two == 2));
  assert (three == 3);
}

int first (int [[1]] arr) {
  return arr[0];
}

void main () {
  int [[1]] brr (5) = 0;
  assert (all(brr == 0));
  int [[1]] t = constant ();
  assert (size(t) == 42);
  assert (all(t == 42));
  assert (all(double(t) == 84));
  assert (first(t) == 42);
  int t1 = 1;
  int [[1]] t2 (2) = 2;
  int t3 = 3;
  order (t1, t2, t3);
}
