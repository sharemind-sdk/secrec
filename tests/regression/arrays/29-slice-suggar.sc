bool all (bool [[1]] arr) {
  for (uint i = 0; i < size(arr); ++ i)
    if (!arr[i])
      return false;
  return true;
}

int sum (int [[1]] arr) {
  uint s = size(arr);
  int sum = 0;
  for (uint i = 0; i < s; ++ i)
    sum += arr[i];
  return sum;
}

void main () {
  int [[1]] arr (3) = 0;

  arr[:] = 1;
  assert (size(arr) == 3);
  assert (all(arr == 1)); 

  arr[1:] = 0;
  assert (arr[0] == 1);
  assert (arr[1] == 0);
  assert (arr[2] == 0);

  arr[:2] = 5;
  assert (arr[0] == 5);
  assert (arr[1] == 5);
  assert (arr[2] == 0);

  arr[0] = 1; arr[1] = 2; arr[2] = 3;
  assert (sum(arr[:]) == 6);
  assert (sum(arr[1:]) == 5);
  assert (sum(arr[:2]) == 3);
}
