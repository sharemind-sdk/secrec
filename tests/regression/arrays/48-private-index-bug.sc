kind additive3pp {
    type uint { public = uint };
}

domain pd additive3pp;

pd uint[[1]] operator + (pd uint[[1]] x, pd uint[[1]] y) {
    return declassify (x) + declassify (y);
}

void main () {
    pd uint [[1]] db (1);
    pd uint sum = 0;
    sum += db[0];
}
