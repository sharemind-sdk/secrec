template<type T>
struct foo {
    foo<T> x;
}

void main() {
    foo<int> x;
}
