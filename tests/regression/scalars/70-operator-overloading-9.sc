kind shared3p;
domain s3p shared3p;

s3p int operator + (s3p int x, int y) {
    return 42;
}

void main () {
    s3p int x;
    assert (declassify (x + x) != 42);
}
