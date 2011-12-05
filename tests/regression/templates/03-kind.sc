
kind a3p;

template <domain dom : a3p>
int unclassify (dom int x) {
    return 0;
}

domain p1 a3p;
domain p2 a3p;

void main () {
    p1 int x;
    p2 int y;
    unclassify (x);
    unclassify (y);
}
