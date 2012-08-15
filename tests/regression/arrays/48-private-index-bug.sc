kind additive3pp;
domain sharemind_test_pd additive3pp;
void main () {
    sharemind_test_pd uint [[1]] db (1);
    sharemind_test_pd uint sum = 0;
    sum += db[0];
}
