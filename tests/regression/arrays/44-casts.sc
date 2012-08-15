bool all (bool [[1]] arr) {
  for (uint i = 0; i < size (arr); ++ i)
    if (!arr[i]) return false;
  return true;
}

void simple_tests () {
  bool [[1]] b (5);
  int [[1]] i (5);
  uint16 [[1]] u (5);

  b = false; i = 1; u = 2;
  b = (bool)   b;
  i = (int)    b;
  u = (uint16) b;
  assert (all (i == 0));
  assert (all (u == (uint16) 0));

  b = false; i = 1; u = 2;
  b = (bool)   i;
  i = (int)    i;
  u = (uint16) i;
  assert (all (b == true));
  assert (all (u == (uint16) 1));

  b = false; i = 1; u = 2;
  b = (bool)   u;
  i = (int)    u;
  u = (uint16) u;
  assert (all (b == true));
  assert (all (i == 2));
}

void overflow_tests () {
  int8 [[1]] i (5);
  uint8 [[1]] u (5);

  u = 255;
  i = (int8) u;
  assert (all (i == - (int8) 1));

  for (uint8 it = 0; it < 255; ++ it) {
    u = it;
    assert (all (((uint8) (int8) u == u)));
  }
}

void main () {
  simple_tests ();
  overflow_tests ();
}
