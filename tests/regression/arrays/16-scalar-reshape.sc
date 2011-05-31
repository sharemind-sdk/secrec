
void main () {
  int [[1]] arr (5) = 0;
  arr = reshape (1, 10);
  assert (size(arr) == 10);
  assert (arr[0] == 1);
}
