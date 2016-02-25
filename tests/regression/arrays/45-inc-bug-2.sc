kind additive3p {
    type int { public = int };
}

domain pd additive3p;

pd int[[1]] operator + (pd int[[1]] x, pd int[[1]] y) { return declassify (x) + declassify (y); }

void main () {
    pd int [[1]] x (1);
    ++ x[0];
    ++ x;
    x[0] ++;
    x ++;
}
