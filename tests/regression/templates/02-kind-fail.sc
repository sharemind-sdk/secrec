kind a3p;
template <domain dom : a3p>
int unclassify (dom int x) {
    return 0;
}

void main () {
    unclassify (0); // unification fail
}
