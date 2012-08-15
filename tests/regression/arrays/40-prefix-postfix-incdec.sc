// prefix and postfix increment and decrement checks for arrays

bool all (bool [[1]] arr) {
  for (uint i = 0; i < size (arr); ++ i)
    if (!arr[i])
      return false;
  return true;
}

void main () {
  int [[1]] arr (10);

  // basic tests
  arr = 0;
  ++ arr;
  assert (all (arr == 1));
  assert (all (-- arr == 0));
  assert (all (arr ++ == 0));
  assert (all (arr == 1));

  // incrementing single position
  arr = 0;
  ++ arr[0];
  assert (arr[0] == 1);
  assert (all (arr[1:] == 0));

  // with dynamic index
  arr = 0;
  for (uint i = 0; i < size (arr); ++ i) {
    arr [i] ++;
  }

  assert (all (arr == 1));
}
