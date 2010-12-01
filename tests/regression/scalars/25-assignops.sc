
void main () {
  int i = 0;

  i += 1;
  assert (i == 1);
  i *= 2;
  assert (i == 2);
  i -= 12;
  assert (i == -10);
  i /= 2;
  assert (i == -5);
  i += 5 + 5;
  assert (i == 5);
}
