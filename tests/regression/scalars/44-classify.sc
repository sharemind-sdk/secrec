// set of legal implicit classifications
kind additive3pp;
domain sharemind_test_pd additive3pp;
void main () {
    sharemind_test_pd int p1;
    public int x;

    sharemind_test_pd bool p2;
    public bool y;

    p1 = x;
    p1 = p1;
    p2 = y;
    p2 = p2;

    p1 = - x;
    p1 = - p1;
    p2 = ¬ y;
    p2 = ¬ p2;

    p1 = p1 + p1;
    p1 = p1 - p1;
    p1 = p1 * p1;
    p1 = p1 + x;
    p1 = p1 - x;
    p1 = p1 * x;
    p1 = x + p1;
    p1 = x - p1;
    p1 = x * p1;
    p1 = x + x;
    p1 = x - x;
    p1 = x * x;

    p2 = p2 && p2;
    p2 = p2 || p2;
    p2 = p2 && y;
    p2 = p2 || y;
    p2 = y && p2;
    p2 = y || p2;
    p2 = y && y;
    p2 = y || y;

    p2 = p1 <  p1;
    p2 = p1 >  p1;
    p2 = p1 <= p1;
    p2 = p1 >= p1;
    p2 = p1 == p1;
    p2 = p1 <  x;
    p2 = p1 >  x;
    p2 = p1 <= x;
    p2 = p1 >= x;
    p2 = p1 == x;
    p2 = x <  p1;
    p2 = x >  p1;
    p2 = x <= p1;
    p2 = x >= p1;
    p2 = x == p1;
    p2 = x <  x;
    p2 = x >  x;
    p2 = x <= x;
    p2 = x >= x;
    p2 = x == x;
}
