kind a3p {
    type int { public = int };
}
domain private a3p;
void main () {
  private int [[1]] arr (5) = 0;
  arr = reshape (1, size(arr));
  assert (size(arr) == 5);
}
