@deprecated
void foo() {
    print("foo");
}

template<type T>
@deprecated
void bar(T x) {
    print("bar");
}

@deprecated("THIS IS GETTING OLD")
void foo2() {
    print("foo2");
}

template<type T>
@deprecated( "THIS IS GETTING EVEN OLDER" )
void bar2(T x) {
    print("bar2");
}

void main() {
    uint x;
    foo();
    bar(x);
    foo2();
    bar2(x);
}
