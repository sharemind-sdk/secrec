kind s3p {
    type int64 { public = int64 };
    type uint32 { public = uint32 };
    type uint8 { public = uint8 };
}

domain pd s3p;

pd uint8[[1]] cast(pd int64[[1]] x) {
    pd uint8[[1]] res(size(x)) = 1;
    return res;
}

template<domain D : s3p, type T1, type T2>
D T2[[1]] cast(D T1[[1]] x) {
    D T2[[1]] res(size(x)) = 42;
    return res;
}

void main() {
    pd int64 x1;
    pd int64[[1]] x2(2);
    pd int64[[2]] x3(2, 2);

    assert(declassify((uint32) x1) == 42);
    assert(declassify((uint32) x2)[0] == 42);
    assert(declassify((uint32) x3)[0, 0] == 42);

    // Prefer non-templated
    assert(declassify((uint8) x1) == 1);
}
