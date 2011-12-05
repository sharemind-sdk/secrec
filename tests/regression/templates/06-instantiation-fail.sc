
template <domain dom>
void hasTypeError (dom int x) {
    if (x) { }
}

kind a3p;
domain private a3p;

void main () {
    private int y;
    hasTypeError (y);
}
