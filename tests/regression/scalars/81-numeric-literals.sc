void main() {
    (bool) (1 :: float32);
    (bool) (1.0 :: float32);

    (bool) (1 + 2);
    (bool) (1 - 2);
    (bool) (1 * 2);
    (bool) (1 / 2);

    (bool) (1.0 + 2);
    (bool) (1.0 - 2);
    (bool) (1.0 * 2);
    (bool) (1.0 / 2);

    (bool) (1 + 2.0);
    (bool) (1 - 2.0);
    (bool) (1 * 2.0);
    (bool) (1 / 2.0);

    (bool) (1.0 + 2.0);
    (bool) (1.0 - 2.0);
    (bool) (1.0 * 2.0);
    (bool) (1.0 / 2.0);

    assert (1 > 0);
    assert (1.0 > 0.0);
    assert (1 > 0.0);
    assert (1.0 > 0);

    assert (1 + 0 > 0);
    assert (1 + 0 > 0.0);
    assert (1.0 + 0.0 > 0.0);
    assert (1 + 0.0 > 0.0);
    assert (1.0 + 0 > 0.0);

    assert ((1 + 2) + 3 > 0.0);
    assert (1 + (2 + 3) > 0);
    assert (1.0 + (2 + 3) > 0.0);
    assert (1 + (2.0 + 3) > 0);
}
