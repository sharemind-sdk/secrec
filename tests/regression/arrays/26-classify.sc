
void main () {
  private int [[1]] carr (5) = 1;
  int [[1]] arr = declassify (carr);
  assert (size(arr) == 5);
  assert (arr[0] == 1);
}
