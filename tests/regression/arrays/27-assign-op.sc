
void main () {
  int [[1]] arr (5) = 0;
  arr += 1;
  assert (size(arr) == 5);
  assert (arr[0] == 1);
  assert (arr[4] == 1);
}
