
kind mykind {
    type mytype { public = int };
}

domain pd mykind;

template <domain D : mykind>
D mytype operator * (D mytype x, int y) {
    return declassify (x) * y;
}

template <domain D, type T>
int proc (D T x) {
    return 42;
}

void main() {
    pd mytype x1;
    pd mytype x2 = 1337;
    pd mytype[[1]] x3 = {1, 2, 3};
    float64 y;

    assert (declassify (x1) == 0);
    assert (declassify (x2) == 1337);
    assert (declassify (x3)[2] == 3);
    assert (declassify (x2 * 2) == 2674);
    assert (proc (x1) == 42);
}
