kind s3p {
    type int64 { public = int64 };
    type uint32 { public = uint32 };
}

domain pd s3p;

pd uint32 cast(pd int64 x) {
    pd uint32 res;
    return res;
}

void main() {
    pd int64 x;
    pd uint32 y = (uint32) x;
}
