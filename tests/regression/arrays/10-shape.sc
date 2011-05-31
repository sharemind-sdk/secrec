
void main () {
  int [[3]] arr (1, 2, 3);
  assert (size(shape(arr)) == 3);
  assert (shape(arr)[0] == 1);
  assert (shape(arr)[1] == 2);
  assert (shape(arr)[2] == 3);

  int [[1]] brr (100);
  assert (shape(brr)[0] == 100);

  int crr;
  assert (size(crr) == 1);
  assert (size(shape(crr)) == 0);
}
