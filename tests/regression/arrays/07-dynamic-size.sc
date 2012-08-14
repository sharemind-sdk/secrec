void main () {
  uint s = 10;
  int [[1]] t (s) = 1;
  assert (size(t) == s);
}
