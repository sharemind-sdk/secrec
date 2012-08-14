
bool all (bool [[1]] arr) {
  for (uint i = 0; i < size(arr); ++ i)
    if (!arr[i])
      return false;
  return true;
}

void main () {
  int [[2]] mat (3, 3);
  int [[1]] arr (9);

  mat[0,0] = 0; mat[0,1] = 1; mat[0,2] = 2;
  mat[1,0] = 3; mat[1,1] = 4; mat[1,2] = 5;
  mat[2,0] = 6; mat[2,1] = 7; mat[2,2] = 8;

  for (uint i = 0; i < 9; ++ i) arr[i] = (int) i;

  int [[1]] t = reshape(mat, 9);
  assert (all (t == arr));
}
