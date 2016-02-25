kind a3p {
    type int { public = int };
}

template <domain dom : a3p>
int unclassify (dom int x) {
    return 0;
}

void main () {
    unclassify (0); // unification fail
}
