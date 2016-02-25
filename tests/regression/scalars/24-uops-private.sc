kind additive3pp {
    type bool { public = bool };
    type int { public = int };
}

template <domain D : additive3pp>
D int operator - (D int x) {
    return - declassify (x);
}

domain sharemind_test_pd additive3pp;

void main () {
  sharemind_test_pd bool t = true;
  sharemind_test_pd bool f = false;
  sharemind_test_pd int one = 1;
  assert (declassify (!f));
  assert (declassify (!!t));
  assert (declassify (-one) == -1);
  assert (declassify (- -one) == 1);
}
