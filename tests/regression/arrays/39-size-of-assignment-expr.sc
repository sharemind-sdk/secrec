void main () {
  int [[1]] arr (10);
  assert (size (arr[1:9] = 1) == 8);
  assert (arr[1] == 1);
  assert (arr[0] == 0);
  assert (arr[9] == 0);
}
