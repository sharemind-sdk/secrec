import common;
void main () {
    int n = 20;
    sharemind_test_pd uint32 [[1]] xu32 (n) = 40, yu32 (n)= 2;
    sharemind_test_pd uint64 [[1]] xu64 (n)= 40, yu64 (n)= 2;
    sharemind_test_pd uint16 [[1]] xu16 (n) = 40, yu16 (n) = 2;
    sharemind_test_pd uint8 [[1]] xu8 (n)= 40, yu8 (n) = 2;
    sharemind_test_pd uint64 au64 = 40, bu64 = 2;
    sharemind_test_pd uint32 au32 = 40, bu32 = 2;
    sharemind_test_pd uint16 au16 = 40, bu16 = 2;
    sharemind_test_pd uint8 au8 = 40, bu8 = 2;

    assert (declassify (au8 * bu8) == (80 :: uint8));
    assert (declassify (au16 * bu16) == (80 :: uint16));
    assert (declassify (au32 * bu32) == (80 :: uint32));
    assert (declassify (au64 * bu64) == (80 :: uint64));
    assert (all (declassify (xu8 * yu8) == (80 :: uint8)));
    assert (all (declassify (xu16 * yu16) == (80 :: uint16)));
    assert (all (declassify (xu32 * yu32) == (80 :: uint32)));
    assert (all (declassify (xu64 * yu64) == (80 :: uint64)));
}
