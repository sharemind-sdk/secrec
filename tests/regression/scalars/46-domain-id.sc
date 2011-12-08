kind a3p;
domain d1 a3p;
domain d2 a3p;

void main () {
    assert (__domainid (d1) != __domainid (d2));
    assert (__domainid (d1) == __domainid (d1));
    assert (__domainid (d2) == __domainid (d2));
}
