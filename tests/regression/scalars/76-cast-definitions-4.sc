kind s3p {
    type int64 { public = int64 };
    type uint32 { public = uint32 };
}

domain pd s3p;

void main() {
    pd int64[[1]] x;
    pd uint32[[1]] y = (uint32) x;
}
