void main () {
    int [[2]] arr (2, 2);
    arr = ++ arr [:,0];
    arr = arr [0,:] ++;
    arr = -- arr [:,1];
    arr = arr [1,:] --;
    assert (true);
}
