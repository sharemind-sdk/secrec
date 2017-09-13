kind shared3p {
    type uint64 { public = uint64 };
}

domain pd_shared3p shared3p;

template<type T>
struct foo { T x; }

template<domain D, type T>
foo<T> foofun(D T x) {
    public foo<T> res;
    res.x = declassify(x);
    return res;
}

void main() {
    pd_shared3p uint proxy;
    foo<uint> x = foofun(proxy);
}
