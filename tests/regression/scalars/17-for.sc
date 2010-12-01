/**
 * simple for loop tests
 */
void main () {
  int i = 0;

  // regular for loop
  for (i = 0; i < 10; i = i + 1) {
    assert (true);
  }

  assert (i == 10);

  // no step
  for (i = 10; false; ) {
    assert (i == 10);
  }

  // no init
  i = 0;
  for ( ; i < 10; i = i + 1) {
    assert (i < 10);
  }

  // no step
  for (i = 0; ; i = i + 1) {
    if (i >= 10) break;
  }

  assert (i == 10);
}
