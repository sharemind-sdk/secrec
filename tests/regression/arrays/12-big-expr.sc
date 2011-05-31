
void main () {
  int [[1]] a (10) = 1;
  int [[1]] b (10) = 1;
  int [[1]] c (10) = 1;
  int [[1]] r = c * (((a + b) * c - a) + b + b + b);
  assert (size(r) == 10);
  assert (r[0] == 4);
}
