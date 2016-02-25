// set of legal implicit classifications
kind additive3pp {
    type uint { public = uint };
    type bool { public = bool };
}

domain pd additive3pp;

pd uint operator - (pd uint x) { return - declassify (x); }
pd bool operator ! (pd bool x) { return ! declassify (x); }
pd uint operator + (pd uint x, pd uint y) { return declassify (x) + declassify (y); }
pd uint operator - (pd uint x, pd uint y) { return declassify (x) - declassify (y); }
pd uint operator * (pd uint x, pd uint y) { return declassify (x) * declassify (y); }
pd bool operator & (pd bool x, pd bool y) { return declassify (x) & declassify (y); }
pd bool operator | (pd bool x, pd bool y) { return declassify (x) | declassify (y); }
pd bool operator < (pd uint x, pd uint y) { return declassify (x) < declassify (y); }
pd bool operator > (pd uint x, pd uint y) { return declassify (x) > declassify (y); }
pd bool operator <= (pd uint x, pd uint y) { return declassify (x) <= declassify (y); }
pd bool operator >= (pd uint x, pd uint y) { return declassify (x) >= declassify (y); }
pd bool operator == (pd uint x, pd uint y) { return declassify (x) == declassify (y); }

void main () {
    pd uint p1;
    public uint x;

    pd bool p2;
    public bool y;

    p1 = x;
    p1 = p1;
    p2 = y;
    p2 = p2;

    p1 = - x;
    p1 = - p1;
    p2 = ! y;
    p2 = ! p2;

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

    p2 = p2 & p2;
    p2 = p2 | p2;
    p2 = p2 & y;
    p2 = p2 | y;
    p2 = y & p2;
    p2 = y | p2;
    p2 = y & y;
    p2 = y | y;

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
