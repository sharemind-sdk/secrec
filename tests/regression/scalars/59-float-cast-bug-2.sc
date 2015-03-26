void main() {
    float64 a = (float64)(18446744073709551615::uint);
    float64 b = 18446744073709551615;
    assert(a == b);
}
