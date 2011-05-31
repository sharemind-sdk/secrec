
void main () {
  int [[1]] l (5) = 1;
  l = -l;
  assert (size(l) == 5);
  assert (l[0] == -1);
  assert (l[1] == -1);

  bool [[1]] b (2) = true;
  b = !b;
  assert (size(b) == 2);
  assert (b[0] == false);
  assert (b[1] == false);
}
