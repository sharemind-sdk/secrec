template <domain dom>
void do_nothing1 (dom int x) { }

template <domain dom>
void do_nothing2 (dom int n) { }


kind a3p;
domain private a3p;

void main () {
    private int z;
    do_nothing1 (z);
    do_nothing2 (13);
}
