void main () {
    int [[2]] arr (2, 2);
    int [[2]] brr;
    brr = ++ arr [:,0];
    brr = arr [0,:] ++;
    brr = -- arr [:,1];
    brr = arr [1,:] --;
    brr = ++ arr [:,:];
    brr = arr [:,:] ++;
    brr = -- arr [:,:];
    brr = arr [:,:] --;
    assert (true);
}
