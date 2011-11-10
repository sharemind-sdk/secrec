kind a3p;
domain private a3p;
void main () {
  private bool t = true;
  private bool f = false;

  assert (declassify ((t && t) == t));
  assert (declassify ((t && f) == f));
  assert (declassify ((f && t) == f));
  assert (declassify ((f && f) == f));

  assert (declassify ((t || t) == t));
  assert (declassify ((t || f) == t));
  assert (declassify ((f || t) == t));
  assert (declassify ((f || f) == f));

  assert (declassify ((t == t) == t));
  assert (declassify ((t == f) == f));
  assert (declassify ((f == t) == f));
  assert (declassify ((f == f) == t));

  assert (declassify ((t != t) == f));
  assert (declassify ((t != f) == t));
  assert (declassify ((f != t) == t));
  assert (declassify ((f != f) == f));
}
