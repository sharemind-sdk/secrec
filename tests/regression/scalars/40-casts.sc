// NB! will fail for now as we only support
// security type casts for now.
void main () {
  int a; int b; int c;
  b = (int) (int) c;
  b = (int) ((int) c);
  a = (int) b;
  assert (true);
  return;
}
