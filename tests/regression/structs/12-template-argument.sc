kind additive3p {
    type bool { public = bool };
    type uint { public = uint };
}
domain pd_a3p additive3p;

template <domain D : additive3p>
D uint operator * (D uint x, D uint y) {
    return declassify (x) * declassify (y);
}

template <domain D : additive3p>
D uint operator + (D uint x, D uint y) {
    return declassify (x) + declassify (y);
}

template <domain D : additive3p>
D bool operator == (D uint x, D uint y) {
    return declassify (x) == declassify (y);
}

template <domain D>
struct point {
    D uint x;
    D uint y;
}

template <domain D>
D uint sqrLen (point<D> p) {
    return p.x*p.x + p.y*p.y;
}

template <domain D>
point<D> get () {
    public point<D> result;
    result.x = 1;
    result.y = 1;
    return result;
}

void main () {
    public point<public> p1 = get ();
    public point<pd_a3p> p2 = get ();
    uint l1 = sqrLen (p1);
    pd_a3p uint l2 = sqrLen (p2);
    assert (l1 == 2);
    assert (declassify (l2 == 2));
    return;
}
