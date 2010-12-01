
int add (int x, int y) {
  if (y == 0) return x;
  return (1 + add(x, y - 1));
}

void main () {
  int x = add (6, 6);
  assert (x == 12);
}
