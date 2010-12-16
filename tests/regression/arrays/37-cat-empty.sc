
// cat(empty, x) == x == cat(x, empty)
void main () {
  int [[1]] arr;
  int [[1]] brr;
  arr = cat(arr, reshape(1, 1));
  brr = cat(reshape(1, 1), brr);
  assert (arr[0] == 1);
  assert (brr[0] == 1);
}
