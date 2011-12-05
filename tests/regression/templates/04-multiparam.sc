
template <domain dom>
void foo (dom int x, dom int y) { }


kind a3p;
domain p1 a3p;

void main () {
    p1 int x;
    p1 int y;
       int z;
    foo (x, x);
    foo (x, y);
    foo (z, z);
}
