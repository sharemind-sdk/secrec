kind a3p;

template <domain dom>
void foo (dom int x) {
    assert (false);
}

template <domain dom : a3p>
void foo (dom int x) { }

void foo (public int x) {
    assert (false);
}

domain private a3p;

void main () {
    private int z;
    foo (z);
}
