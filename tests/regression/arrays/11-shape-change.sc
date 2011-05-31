
void main () {
  int [[1]] arr (1) = 1;
  int [[1]] brr (2) = 2;
  bool b = false;
  if (b) arr = brr;
  assert (size(arr) == 1);
}
