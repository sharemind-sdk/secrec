kind s3p {
    type int64 { public = int64 };
    type uint32 { public = uint32 };
    type foo { public = uint32 };
}

domain pd s3p;

pd uint32[[1]] cast(pd int64[[1]] x) {
    pd uint32[[1]] res(size(x)) = 42;
    return res;
}

pd foo[[1]] cast(pd int64[[1]] x) {
    pd foo[[1]] res(size(x)) = 42;
    return res;
}

void main() {
    pd int[[1]] x1(2);
    pd int x2;
    pd int[[2]] x3(2, 2);

    // Different dimensions
    assert(declassify(((uint32) x1)[0]) == 42);
    assert(declassify((uint32) x2) == 42);
    assert(declassify(((uint32) x3))[0, 0] == 42);

    // User-defined type
    assert(declassify(((foo) x1)[0]) == 42);
    assert(declassify((foo) x2) == 42);
    assert(declassify(((foo) x3))[0, 0] == 42);
}
