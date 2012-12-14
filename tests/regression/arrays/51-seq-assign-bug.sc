void main () {
    uint [[1]] a (1), b (1);
    a[0] = b[0] = 1;
    assert (a[0] == 1);
}
