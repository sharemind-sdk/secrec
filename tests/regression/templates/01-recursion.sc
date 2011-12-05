template <domain dom>
dom int loop (dom int x) {
    if (false) /// to avoid actually recursing
        loop (x);
    return x;
}

kind a3p;
domain private a3p;

void main () {
    private int z;
    loop (z);
    loop (0);
}
