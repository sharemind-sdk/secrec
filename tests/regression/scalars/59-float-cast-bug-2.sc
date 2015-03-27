void main() {
    {
        float32 a = (float32)(4294967295::uint32);
        float32 b = 4294967295;
        assert(a == b);
    }
    {
        float64 a = (float64)(18446744073709551615::uint);
        float64 b = 18446744073709551615;
        assert(a == b);
    }
}
