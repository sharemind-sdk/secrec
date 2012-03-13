module common;

kind additive3pp;
domain sharemind_test_pd additive3pp;

bool all (bool [[1]] arr) {
    for (int i = 0; i < size (arr); ++ i) {
        if (! arr[i]) {
            return false;
        }
    }

    return true;
}
