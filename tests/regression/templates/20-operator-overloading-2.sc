
kind shared3p;

domain pd_shared3p shared3p;

template<domain D1, domain D2>
D1 int operator + (D2 int x, D2 int y) {
    return x;
}

void main () {}
