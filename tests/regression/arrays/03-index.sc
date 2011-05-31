
void main () {
  int [[2]] arr (5, 5);
  int [[1]] brr (4) = 1;
  assert (arr[0, 0] == 0);
  assert (arr[1, 4] == 0);
  assert (brr[3] == 1);
  assert (brr[0] == 1);
}
