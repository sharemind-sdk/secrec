template<type T>
T f() {
    return 1;
}

template<type T>
void b(T x) {
    print(x);
}

void main () {
    uint x = f();
    b(f());
}
