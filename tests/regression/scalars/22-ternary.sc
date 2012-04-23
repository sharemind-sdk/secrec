kind additive3pp;
domain sharemind_test_pd additive3pp;

int val = 0;

void load_one () { val = 1; }

void load_two () { val = 2; }

void main () {
  bool t = true;
  bool f = false;

  assert ( (t ? 11 : 12) == 11 );
  assert ( (f ? 13 : 14) == 14 );

  sharemind_test_pd int one = 1;
  sharemind_test_pd int two = 2;
  assert ( declassify ((t ? one : two) == one) );
  assert ( declassify ((f ? one : two) == two) );

  t ? load_one() : load_two();
  assert (val == 1);

  f ? load_one() : load_two();
  assert (val == 2);
}
