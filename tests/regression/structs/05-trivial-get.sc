struct vec2d {
    int x;
    int y;
}

struct foo {
    int[[1]] a;
    vec2d    b;
    int8     c;
}

void main () {
    vec2d t1;
    assert (t1.x == 0);
    assert (t1.y == 0);

    foo t2;
    assert (size (t2.a) == 0);
    assert (t2.b.x == 0);
    assert (t2.b.y == 0);
    assert (t2.c == 0);
}
