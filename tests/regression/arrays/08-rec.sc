
int first (int [[1]] arr) {
  return arr[0];
}

void main () {
  int [[1]] arr (10) = 1;
  int t = first (arr);
  assert (t == 1);
}
