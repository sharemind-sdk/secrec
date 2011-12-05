
template <domain dom>
bool even (dom int n) {
    if (n == 0) return true;
    return odd (n - 1);
}

template <domain dom>
bool odd (dom int n) {
    if (n == 0) return false;
    return even (n - 1);
}

void main () {
    assert (odd (1));
    assert (even (4));
    assert (odd (13));
    assert (even (16));
}
