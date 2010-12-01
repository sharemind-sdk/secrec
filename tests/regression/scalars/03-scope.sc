
int glob = 1;
void main () {
  int x = 0;
  { int x; x = 1; assert (x == 1); }
  assert (x == 0);
  { x = 2; }
  assert (x == 2);
  assert (glob == 1);
}
