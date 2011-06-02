void main () {
  int s = 10;

  for (int i = 0; i < 10; ++ i) {
    assert (i >= 0);
    assert (i < 10);
  }

  int count = 0;
  for (int i; i < s; ++ i) {
    ++ count;
  }

  assert (count == s);

  return;
}
