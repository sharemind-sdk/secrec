
void main () {
  int i = 0;
  while (true) {
    if (i > 10) break;
    i = i + 1;
  }

  assert (i == 11);
}
