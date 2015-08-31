void main() {
    // float conversion to integer should round to in the 0 direction.

    float32 x = 1.6;
    assert ((int)x == 1);

    float32 y = -1.6;
    assert ((int)y == -1);
}
