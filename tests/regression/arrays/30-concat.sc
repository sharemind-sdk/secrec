
void main () {
  int [[1]] arr (5) = 0;
  int [[1]] brr (5) = 1;
  int [[1]] crr;

  crr = cat (arr, brr, 0);
  assert (size(crr) == 10);
  assert (crr[0] == 0);
  assert (crr[5] == 1);
}
