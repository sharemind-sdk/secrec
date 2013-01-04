
template <dim D>
void foo (uint [[D]] arr) {
    uint [[D]] brr = arr;
}

void main () {
    foo (0 :: uint);
    foo (reshape (0 :: uint, 1, 2));
}
