
kind a3p;
domain p1 a3p;
domain p2 a3p;

public int foo (int n) { assert (n == 1); return 0; }
p1 int foo (int n) { assert (n == 2); return 0; }
p2 int foo (int n) { assert (n == 3); return 0; }

void main () {
       int x;
    p1 int y;
    p2 int z;
    x = foo (1);
    y = foo (2);
    z = foo (3);
}
