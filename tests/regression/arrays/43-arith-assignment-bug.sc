void main () {
    int [[1]] arr (1);
    arr = 1;
    arr += arr;
    assert (arr [0] == 2);
}
