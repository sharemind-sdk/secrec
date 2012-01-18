// x was not in scope in declaration.
void main () {
    int x = 0, y = x + 1;
    assert (x == 0);
    assert (y == 1);
    return;
}
