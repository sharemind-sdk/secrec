
void main () {
  bool t = true;
  bool f = false;

  assert ((t && t) == t);
  assert ((t && f) == f);
  assert ((f && t) == f);
  assert ((f && f) == f);

  assert ((t || t) == t);
  assert ((t || f) == t);
  assert ((f || t) == t);
  assert ((f || f) == f);

  assert ((t == t) == t);
  assert ((t == f) == f);
  assert ((f == t) == f);
  assert ((f == f) == t);

  assert ((t != t) == f);
  assert ((t != f) == t);
  assert ((f != t) == t);
  assert ((f != f) == f);
}
