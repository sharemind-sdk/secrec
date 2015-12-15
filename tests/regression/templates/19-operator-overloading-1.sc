
kind shared3p;
domain s3p shared3p;

kind shared2p;
domain s2p shared2p;

int operator + (int x, int y) {
    return 1;
}

template <type T>
T operator + (T x, T y) {
    return 2;
}

template <type T, domain D>
D T operator + (D T x, D T y) {
    return 3;
}

template <type T, domain D : shared3p>
D T operator + (D T x, D T y) {
    return 4;
}

template <type T, domain D : shared3p>
D T operator + (D T x, T y) {
    return 5;
}

void main () {
    int x1;
    uint x2;
    s2p int x3;
    s3p int x4;

    assert (x1 + x1 == 1);
    assert (x2 + x2 == 2);
    assert (declassify (x3 + x3) == 3);
    assert (declassify (x4 + x4) == 4);
    assert (declassify (x4 + x1) == 5);
}
