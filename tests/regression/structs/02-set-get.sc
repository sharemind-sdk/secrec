struct vec2d {
    int x;
    int y;
}

struct weird {
    vec2d u;
    vec2d v;
}

void main () {
    vec2d u;

    assert (u.x == 0);
    u.x = 1;
    assert (u.x == 1);

    weird x;
    assert (w.u.x == 0);
    w.u.x = 1;
    assert (w.u.x == 1);
}
