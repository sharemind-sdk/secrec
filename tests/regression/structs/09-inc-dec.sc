struct wrap {
    int x;
}

struct arr_t {
    int[[1]] x;
}

void main () {
    wrap t;

    assert (t.x ++ == 0);
    assert (t.x -- == 1);
    assert (++ t.x == 1);
    assert (-- t.x == 0);

    arr_t a;
    a.x = {1};

    a.x ++;
    a.x --;
    ++ a.x;
    -- a.x;

    a.x[0] ++;
    a.x[0] --;
    ++ a.x[0];
    -- a.x[0];
}
