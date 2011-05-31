
void main () {
  int [[1]] arr (5) = 0;
  int i;

  for (i = 0; i < 5; i += 1) {
    arr[i:5] += 1;
  }

  assert (arr[0] == 1);
  assert (arr[1] == 2);
  assert (arr[2] == 3);
  assert (arr[3] == 4);
  assert (arr[4] == 5);
}
