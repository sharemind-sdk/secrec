// parsing check for type casts
void main () {
  int a; int b; int c;
  b = (int) (int) c;
  b = (int) ((int) c);
  a = (int) b;
  assert (true);
  return;
}
