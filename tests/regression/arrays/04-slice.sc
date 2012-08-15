
void main () {
  int [[2]] arr (5, 5);
  uint t1 = size(arr[1,0:3]);
  uint t2 = size(arr[1:5,0]);
  uint t3 = size(arr[0:1,4:5]);
  assert (size(arr[0:4,0:4]) == 16);
  assert (t1 == 3);
  assert (t2 == 4);
  assert (t3 == 1);
}
