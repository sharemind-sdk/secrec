
kind additive3pp;
domain p1 additive3pp;
domain p2 additive3pp;

// by domain type
public int foo (int n) { assert (n == 1); return 0; }
p1 int foo (int n) { assert (n == 2); return 0; }
p2 int foo (int n) { assert (n == 3); return 0; }

// by data type
bool get (int n) { assert (n == 4); return false; }
int get (int n) { assert (n == 5); return 0; }

// by dimensionality type
int arr (int n) { assert (n == 6); return 0; }
int [[1]] arr (int n) { assert (n == 7); return reshape (0, 1); }

void main () {
    int x1 = foo (1);
    p1 int y1 = foo (2);
    p2 int z1 = foo (3);
    bool x2 = get (4);
    int y2 = get (5);
    int x3 = arr (6);
    int [[1]] y3 = arr (7);
    return;
}
