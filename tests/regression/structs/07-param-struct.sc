struct vec {
    int x;
    int y;
}

void check (vec v) {
    assert (v.x == 1);
    assert (v.y == 2);
}

void main () {
    vec v;
    v.x = 1;
    v.y = 2;
    check (v);
}
