void main () {
  int [[1]] F (2);
  F[1] = 1;
  F = cat (F, F); // 0 1 0 1
  assert (F[1] == 1);
}
