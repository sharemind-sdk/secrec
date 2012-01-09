kind a3p;

template <domain d1 : a3p, domain d2 : a3p >
d2 int reclassify (d1 int x) { return 0; }

template <domain d : a3p >
d int reclassify (d int x) { return x; }

template <domain d : a3p >
d int classify_ (public int x) { d int out; return out; }

domain private a3p;
domain protected a3p;

void main () {
    private int x;
    protected int y;
    x = reclassify (x);
    x = reclassify (y);
    y = reclassify (x);
    y = reclassify (y);
    x = classify_ (0);
}

