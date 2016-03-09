kind shared3p {
    type int { public = int };
}
domain s3p shared3p;

s3p int operator + (s3p int x, s3p int y) {
    return declassify (x) + declassify (y) + 1;
}

void main () {
    s3p int x;
    assert (declassify (x += 1) == 2);
}
