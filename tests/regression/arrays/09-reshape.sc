
void main () {
  int [[1]] t (4) = 0;
  int [[2]] m (2, 2) = 1;
  t = reshape (t, 4);
  assert (size(t) == 4);
  assert (t[0] == 0);

  t = reshape (m, 4);
  assert (size(t) == 4);
  assert (size(shape(m)) == 2);
  assert (t[0] == 1);

  t = reshape (42, 1);
  assert (size(t) == 1);
  assert (t[0] == 42);
}
