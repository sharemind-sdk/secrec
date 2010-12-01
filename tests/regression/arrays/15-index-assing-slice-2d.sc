void main () {
  int n = 5;
  int m = 5;
  int [[2]] mat [n, m] = 0;
  mat[1:(n-1), 1:(m-1)] = 1;
  int i; int j;
  for (i = 0; i < n; i = i + 1) {
    for (j = 0; j < m; j = j + 1) {
      int v = mat [i, j];
      if (i > 0 && i + 1 < n && j > 0 && j + 1 < m) {
        assert (v == 1);
      }
      else {
        assert (v == 0);
      }
    }
  }
}
