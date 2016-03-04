kind s3p {
    type int64 { public = int64 };
}

domain pd s3p;

pd uint32[[1]] cast(pd int64[[1]] x) {
    pd uint32[[1]] res(size(x));
    return res;
}

void main() {}
