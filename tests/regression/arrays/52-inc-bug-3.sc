void main () {
    int [[2]] arr (2, 2);
    int[[1]] brr = (++ arr [0,:]);
    assert (size (brr) == 2);
}
