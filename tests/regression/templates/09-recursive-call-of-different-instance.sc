
kind a3p {
    type int { public = int };
}
domain private a3p;

template <domain dom>
void rec (dom int x) {
    public int y;
    if (false) rec (y);
}

void main () {
    private int z;
    rec (z);
}
