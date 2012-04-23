kind additive3pp;
domain d1 additive3pp;
domain d2 additive3pp;

void main () {
    assert (__domainid (d1) != __domainid (d2));
    assert (__domainid (d1) == __domainid (d1));
    assert (__domainid (d2) == __domainid (d2));
}
