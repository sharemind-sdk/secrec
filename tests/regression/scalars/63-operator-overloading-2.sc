kind shared3p {
    type int { public = int };
}
domain s3p shared3p;

bool all (bool[[1]] vec) {
    uint n = size (vec);
    for (uint i = 0; i<n; ++i) {
        if (!vec[i]) {
            return false;
        }
    }
    return true;
}

s3p int operator + (s3p int x, s3p int y) { return 42; }

s3p int[[1]] operator + (s3p int[[1]] x, s3p int[[1]] y) {
    s3p int[[1]] r(size(x)) = 43;
    return r;
}

void main () {
    s3p int x1;
    int y1;

    // Implicit classify
    assert (declassify (x1 + y1) == 42);

    // Should use built-in instead of classifying both operands
    s3p int z1 = y1 + y1;
    assert (declassify (z1) == 0);

    // Stretch scalar to vector
    s3p int[[1]] y2(2);
    assert (all (declassify (x1 + y2) == 43));

    // Reshape matrix to vector
    s3p int[[2]] y3(2, 2);
    s3p int[[2]] z3 = x1 + y3;
    assert (all (reshape (declassify (z3), size (z3)) == 43));
}
