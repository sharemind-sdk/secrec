
template <type T>
struct nest {
    T x;
}

void main () {
    nest<nest<int> > n;
    assert (n.x.x == 0);
}
