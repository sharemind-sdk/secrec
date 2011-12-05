kind a3p;

template <domain dom : a3p>
void stump (dom int x) {
    assert (false);
}

void stump (public int x) {
    assert (true);
}

void main () {
    stump (42);
}
