void nothing() { }

void foo(int x) { }

void main() {
    shape(nothing());
    cat(nothing(),nothing());
    reshape(nothing(), 0);
    nothing() + nothing();
    - nothing();
    foo(nothing());
    __string_from_bytes(nothing());
    __bytes_from_string(nothing());
    assert(nothing());
    { nothing() };
    declassify(nothing());
    // ++ nothing(); // parse error
    // nothing() ++; // parse error
    // nothing() += 1; // parse error
    strlen(nothing());
    (int) nothing();
    nothing()[0];
}

