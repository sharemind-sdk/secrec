import common;

// Test for declassify/classify identity
void main () {

    // Scalars:
    for (int i = 0; i < 100; ++ i) {
        uint8 iu8 = (uint8) i;
        uint16 iu16 = (uint16) i;
        uint32 iu32 = (uint32) i;
        uint64 iu64 = (uint64) i;
        sharemind_test_pd uint8 ju8 = iu8;
        sharemind_test_pd uint16 ju16 = iu16;
        sharemind_test_pd uint32 ju32 = iu32;
        sharemind_test_pd uint64 ju64 = iu64;
        assert (iu8 == declassify (ju8)); // OK
        assert (iu16 == declassify (ju16)); // OK
        assert (iu32 == declassify (ju32)); // OK
        assert (iu64 == declassify (ju64)); // OK
    }

    // Arrays:
    int n = 10;
    uint8 [[1]] au8 (n);
    uint16 [[1]] au16 (n);
    uint32 [[1]] au32 (n);
    uint64 [[1]] au64 (n);
    sharemind_test_pd uint8 [[1]] cau8 = au8;
    sharemind_test_pd uint16 [[1]] cau16 = au16;
    sharemind_test_pd uint32 [[1]] cau32 = au32;
    sharemind_test_pd uint64 [[1]] cau64 = au64;
    assert (all (declassify (cau8) == au8)); 
    assert (all (declassify (cau16) == au16)); 
    assert (all (declassify (cau32) == au32)); 
    assert (all (declassify (cau64) == au64)); 
    return;
}
