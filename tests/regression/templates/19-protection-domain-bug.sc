kind shared3p;
domain pd_shared3p shared3p;

template <type T>
void foo(nonexistant T x) {}

void main() {
    pd_shared3p uint x;
    foo(x);
}
