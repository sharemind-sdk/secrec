void syntax_tests () {
  int i;

  (int) i;
  (int) (uint) i; // right associative

  int16 j;
  (int8) j + (int8) j; // (int8) (j + (int8) j) is type error
  (int8) - j;
}

void simple_tests () {
  bool b;
  int i;
  uint16 u;

  b = false; i = 1; u = 2;
  b = (bool)   b;
  i = (int)    b;
  u = (uint16) b;
  assert (i == 0);
  assert (u == (uint16) 0);

  b = false; i = 1; u = 2;
  b = (bool)   i;
  i = (int)    i;
  u = (uint16) i;
  assert (b == true);
  assert (u == (uint16) 1);

  b = false; i = 1; u = 2;
  b = (bool)   u;
  i = (int)    u;
  u = (uint16) u;
  assert (b == true);
  assert (i == 2);
}

void overflow_tests () {
  int8 i;
  uint8 u;

  u = 255;
  i = (int8) u;
  assert (i == - (int8) 1);

  for (uint8 it = 0; it < 255; ++ it) {
    assert ((uint8) (int8) it == it);
  }
}

void main () {
  syntax_tests ();
  simple_tests ();
  overflow_tests ();
}
