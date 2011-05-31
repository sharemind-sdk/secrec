
void main () {
  int [[1]] empty_arr;
  int [[1]] arr (100);
  int [[2]] mat (5, 5);
  assert (size(empty_arr) == 0);
  assert (size(arr) == 100);
  assert (size(mat) == 25);
}
