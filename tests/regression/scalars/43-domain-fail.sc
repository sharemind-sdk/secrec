kind a3p;
domain p1 a3p;
domain p2 a3p;
void main () {
    p1 int x1;
    p1 int x2;
    p2 int y1;
    x1 = x2; // OK
    x1 = y1; // FAIL
}
