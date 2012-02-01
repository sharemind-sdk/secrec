kind test;
domain private test;
domain protected test;

template <domain T1>
int check (T1 int val) {
    return 0;
}

template <domain T1 : test >
int check (T1 int val) {
    return 1;
}

int check (private int val) {
    return 2;
}

template <domain T1>
T1 int get () {
    T1 int out;
    return out;
}

void main () {
    assert (check (0) == 0);
    assert (check (get () :: private) == 2);
    assert (check (get () :: protected) == 1);
}
