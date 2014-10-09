bool all (bool[[1]] arr) {
    for (uint i = 0; i < size (arr); ++ i) {
        if (! arr[i])
            return false;
    }

    return true;
}
void main () {
    string s = "hello, world!";
    string e = "";

    assert (strlen (s) == 13);
    assert (strlen ("foo") == 3);
    assert (strlen ("") == 0);
    assert (strlen (e) == 0);

    uint8[[1]]
        t1 = __bytes_from_string (s),
        t2 = __bytes_from_string ("foo"),
        t3 = __bytes_from_string (""),
        t4 = __bytes_from_string (e);
    assert (size (t1) == 13);
    assert (size (t2) == 3);
    assert (size (t3) == 0);
    assert (size (t4) == 0);
    assert (all (t1 == {104,101,108,108,111,44,32,119,111,114,108,100,33}));
    assert (all (t2 == {102,111,111}));
}
