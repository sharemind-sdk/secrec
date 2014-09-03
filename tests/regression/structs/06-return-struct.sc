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

void main () {
    vec t = get ();
    assert (t.x == 1);
    assert (t.y == 2);
}
