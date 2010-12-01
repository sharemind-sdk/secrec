
void main () {
  bool test = true;
  int i = 1;
  while (i < 2) {
    i = i + 1;
    if (true) {continue; }
    test = false;
  }

  assert (test);
}

