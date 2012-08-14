bool all (bool [[2]] arr) {
  uint n = size(arr);
  bool [[1]] flat = reshape(arr, n);
  for (uint i = 0; i < n; ++ i)
    if (!flat[i])
      return false;
  return true;
}

void main () {
  int [[2]] arr (2, 2) = 1;
  arr [0, 0] = 0;
  assert (all(arr[ :1, :1] == 0));
  assert (all(arr[1: ,1: ] == 1));
}
