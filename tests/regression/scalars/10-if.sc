
void main () {
  bool t = true;
  if (t) { } else { assert (false); }
  int x = 0;
  if (true) x = 1;
  assert (x == 1);
  if (false) x = 0;
  assert (x == 1);
  if (false) { assert (false); }
}
