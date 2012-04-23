kind additive3pp;
domain sharemind_test_pd additive3pp;
void main () {
  sharemind_test_pd bool t = true;
  sharemind_test_pd bool f = false;
  sharemind_test_pd int one = 1;
  sharemind_test_pd int none = -1;
  assert (declassify (!f));
  assert (declassify (!!t));
  assert (declassify (-one == none));
  assert (declassify (- -one == one));
}
