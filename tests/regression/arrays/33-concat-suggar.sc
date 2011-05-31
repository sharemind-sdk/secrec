
void main () {
  int [[1]] arr (5) = 0;
  int [[1]] brr (5) = 1;
  int [[1]] crr;

  crr = cat (arr, brr);
  assert (size(crr) == 10);
}
