kind a3p;

template <dom>
void foo (dom int x) {
    assert (false);
}

template <dom : a3p>
void foo (dom int x) { }

void foo (public int x) {
    assert (false);
}

domain private a3p;

void main () {
    private int z;
    foo (z);
}
