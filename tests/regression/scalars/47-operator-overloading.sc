kind additive3pp {
    type bool { public = bool };
    type int { public = int };
}
domain private additive3pp;

private int operator +  (private int x, private int y)  { return  0; }
private int operator +  (private int x, int y)          { return  1; }
private int operator +  (int x, private int y)          { return  2; }

private int operator -  (private int x, private int y)  { return  3; }
private int operator *  (private int x, private int y)  { return  4; }
private int operator /  (private int x, private int y)  { return  5; }
private int operator %  (private int x, private int y)  { return  6; }
private int operator & (private int x, private int y)   { return 7; }
private int operator | (private int x, private int y)   { return 8; }

private bool operator ! (private bool b) { return false; }
private int  operator - (private int x)  { return 43; }

private bool operator == (private int x, private int y) { return false; }
private bool operator <  (private int x, private int y) { return true; }
private bool operator <= (private int x, private int y) { return false; }
private bool operator >  (private int x, private int y) { return true; }
private bool operator >= (private int x, private int y) { return false; }

void main () {
    private int x;
    private bool b;

    assert (declassify (x + x) == 0);
    assert (declassify (x + 0) == 1);
    assert (declassify (1 + x) == 2);
    assert (declassify (x - x) == 3);
    assert (declassify (x * x) == 4);
    assert (declassify (x / x) == 5);
    assert (declassify (x % x) == 6);
    assert (declassify (x & x) == 7);
    assert (declassify (x | x) == 8);

    assert (! declassify (x == x));
    assert (declassify (x < x));
    assert (! declassify (x <= x));
    assert (declassify (x > x));
    assert (! declassify (x >= x));

    assert (declassify (! b) == false);
    assert (declassify (- x) == 43);
}
