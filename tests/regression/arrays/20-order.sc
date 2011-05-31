
bool all (bool [[1]] arr) {
  int i;
  for (i = 0; i < size(arr); i += 1)
    if (!arr[i])
      return false;
  return true;
}

void main () {
  int [[2]] mat (3, 3);
  int [[1]] arr (9);
  int i;

  mat[0,0] = 0; mat[0,1] = 3; mat[0,2] = 6;
  mat[1,0] = 1; mat[1,1] = 4; mat[1,2] = 7;
  mat[2,0] = 2; mat[2,1] = 5; mat[2,2] = 8;

  for (i = 0; i < 9; i += 1) arr[i] = i;

  int [[1]] t = reshape(mat, 9);
  assert (all (t == arr));
}
