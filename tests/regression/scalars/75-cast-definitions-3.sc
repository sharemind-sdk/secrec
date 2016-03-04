kind s3p {
    type int64 { public = int64 };
    type uint32 { public = uint32 };
}

domain pd s3p;

pd uint32[[1]] cast(int64[[1]] x) {
    pd uint32[[1]] res(size(x));
    return res;
}

void main() {
    pd int64[[1]] x;
    pd uint32[[1]] y = (uint32) x;
}
