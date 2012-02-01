bool get (int n) { assert (n == 1); return false; }
int8 get (int n) { assert (n == 2); return 0; }
void main () {
  int8 i = 0 :: int8;
  assert (i == (0 :: int8));
  get (1) :: bool;
  get (2) :: int8;
  return;
}
