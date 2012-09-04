kind additive3pp;
domain sharemind_test_pd additive3pp;

void main () {
  sharemind_test_pd uint [[2]] F_cache (0, 5);
  sharemind_test_pd uint [[2]] z (1, 5);
  cat (F_cache, z);
}
