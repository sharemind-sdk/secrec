import common;
void main () {
    int n = 10;
    sharemind_test_pd bool [[1]] aub (n);
    sharemind_test_pd uint8 [[1]] au8 (n);
    sharemind_test_pd uint16 [[1]] au16 (n);
    sharemind_test_pd uint32 [[1]] au32 (n);
    sharemind_test_pd uint64 [[1]] au64 (n);

    for (int i = 0; i < n; ++ i) {
        aub[i] = (i % 3 == 0);
        au8[i] = (uint8) i;
        au16[i] = (uint16) i;
        au32[i] = (uint32) i;
        au64[i] = (uint64) i;
    }

    for (int i = 0; i < n; ++ i) {
        assert (declassify (aub[i]) == (i % 3 == 0));
        assert (declassify (au8[i]) == (uint8) i);
        assert (declassify (au16[i]) == (uint16) i);
        assert (declassify (au32[i]) == (uint32) i);
        assert (declassify (au64[i]) == (uint64) i);
    }

    int m = 10, k = 5;
    sharemind_test_pd uint8 [[2]] mu8 (m, k);
    sharemind_test_pd uint16 [[2]] mu16 (m, k);
    sharemind_test_pd uint32 [[2]] mu32 (m, k);
    sharemind_test_pd uint64 [[2]] mu64 (m, k);
    for (int i = 0; i < m; ++ i) {
        for (int j = 0; j < k; ++ j) {
            int v = i*k + j;
            mu8[i,j] = (uint8) v;
            mu16[i,j] = (uint16) v;
            mu32[i,j] = (uint32) v;
            mu64[i,j] = (uint64) v;
        }
    }
    
    for (int i = 0; i < m; ++ i) {
        for (int j = 0; j < k; ++ j) {
            int v = i*k + j;
            assert (declassify (mu8[i,j]) == (uint8) v);
            assert (declassify (mu16[i,j]) == (uint16) v);
            assert (declassify (mu32[i,j]) == (uint32) v);
            assert (declassify (mu64[i,j]) == (uint64) v);
        }
    }

}
