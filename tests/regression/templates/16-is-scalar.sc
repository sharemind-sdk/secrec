
bool isScalar (uint scalar) { return true; }

template <dim D>
bool isScalar (uint [[D]] arr) { return false; }

void main () {
    assert (isScalar (0 :: uint));
    assert (! isScalar (reshape (0 :: uint, 10)));
}
