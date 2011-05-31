
void main () {
  int [[1]] a (10) = 1;
  int [[1]] b (10) = 2;
  int t = size(a + b);
  assert (t == 10);
}
