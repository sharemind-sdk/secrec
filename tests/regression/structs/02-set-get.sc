struct vec2d {
    int x;
    int y;
}

struct weird {
    int[[1]] a;
    vec2d    u;
    vec2d    v;
}

void main () {
    vec2d u;
    u.x = 1;
    u.y = 2;
    assert (u.x == 1);
    assert (u.y == 2);

    weird w;
    w.a = {1};
    w.u.x = 1;
    assert (size (w.a) == 1);
    assert (w.a[0] == 1);
    assert (w.u.x == 1);

    w.a[0] = 42;
    assert (w.a[0] == 42);
}
