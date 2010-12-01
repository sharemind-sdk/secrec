
void main () {
  bool t = false;
  do {
    t = true;
  } while (false);
  assert (t);

  int i = 0;
  do {
    assert (i < 10);
    i += 1;
  } while (i < 10);
  assert (i == 10);
}
