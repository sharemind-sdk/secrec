
bool all (bool [[2]] arr) {
  int i; int n = size(arr);
  bool [[1]] flat = reshape(arr, n);
  for (i = 0; i < n; i += 1)
    if (!flat[i])
      return false;
  return true;
}

void main () {
  int [[2]] arr (5, 2) = 0;
  int [[2]] brr (5, 3) = 1;
  int [[2]] crr;

  crr = cat (arr, brr, 1);
  assert (size(crr) == 25);
  assert (shape(crr)[0] == 5);
  assert (shape(crr)[1] == 5);
  assert (all(crr[:, :2] == 0));
  assert (all(crr[:,2: ] == 1));
}
