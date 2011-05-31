
void main () {
  int [[2]] m2 (5, 0);
  cat (m2, reshape (1, 5, 1), 1); /// segfault in LOAD
  assert (true);
}
