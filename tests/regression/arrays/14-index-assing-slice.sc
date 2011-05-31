
void main () {
  int [[1]] arr (3) = 0;
  arr[1:2] = 1;
  arr[2:3] = 2;
  assert (arr[0] == 0);
  assert (arr[1] == 1);
  assert (arr[2] == 2);
}
