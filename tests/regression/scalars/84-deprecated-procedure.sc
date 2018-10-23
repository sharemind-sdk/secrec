@deprecated
void foo() {
    print("foo");
}

template<type T>
@deprecated
void bar(T x) {
    print("bar");
}

void main() {
    uint x;
    foo();
    bar(x);
}
