void main () {
    uint64[[1]] c;
    
    c = true ? {0} : {1, 2};
    assert (size (c) == 1);
    assert (c[0] == 0);

    c = false ? {0} : {1, 2};
    assert (size (c) == 2);
    assert (c[0] == 1);
    assert (c[1] == 2);
}
