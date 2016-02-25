kind a3p {
    type bool { public = bool };
}
domain private a3p;
void main () {
  private bool t = true;
  assert (declassify (t));
}
