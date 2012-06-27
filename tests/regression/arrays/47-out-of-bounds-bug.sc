void main () {
    uint [[1]] x (10);
    x[0] = 0;
    assert (size (x) == 10);
}
