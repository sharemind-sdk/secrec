struct vec {
    int x;
    int y;
}

vec get () {
    vec result;
    result.x = 1;
    result.y = 2;
    return result;
}

struct nontrivial {
    int[[1]] x;
    vec y;
    int8 z;
}

nontrivial getNontrivial () {
    nontrivial r;
    r.x   = {1};
    r.y.x = 2;
    r.y.y = 3;
    r.z   = 4;
    return r;
}

void main () {
    vec t = get ();
    assert (t.x == 1);
    assert (t.y == 2);

    nontrivial s = getNontrivial ();
    assert (s.x[0] == 1);
    assert (s.y.x  == 2);
    assert (s.y.y  == 3);
    assert (s.z    == 4);
}
