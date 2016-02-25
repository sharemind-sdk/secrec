
template <domain dom>
void foo (dom int x, dom int y) { }


kind a3p {
    type int { public = int };
}

domain p1 a3p;

void main () {
    p1 int x;
       int z;
    foo (x, z); // unification fail
}
