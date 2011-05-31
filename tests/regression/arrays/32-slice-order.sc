bool all (bool [[2]] arr) {
  int i; int n = size(arr);
  bool [[1]] flat = reshape(arr, n);
  for (i = 0; i < n; i += 1)
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
