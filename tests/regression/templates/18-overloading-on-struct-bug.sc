
struct get_one_tag { }
struct get_two_tag { }

template <type T>
uint get_value(T dummy, public get_one_tag _t) { return 1; }

template <type T>
uint get_value(T dummy, public get_two_tag _t) { return 2; }

void main () {
    public get_one_tag get_one;
    public get_two_tag get_two;

    assert (get_value(0 :: uint, get_one) == 1);
    assert (get_value(0 :: uint, get_two) == 2);
}
