kind test;

template <domain T1 : test, domain T2>
void bad (T1 int x,  T2 int y) { return; }

template <domain T1, domain T2 : test>
void bad (T1 int x,  T2 int y) { return; }


domain private test;

void main () {
    private int x;
    bad (x, x);
}
