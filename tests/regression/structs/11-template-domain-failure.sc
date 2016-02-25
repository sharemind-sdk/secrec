kind a3p {
    type foo;
}

template <domain D : a3p>
struct test { }

void main () {
    test<public> x;
}
