kind a3p;
domain private a3p;
void main () {
  private bool t = true;
  private bool f = false;
  private int one = 1;
  private int none = -1;
  assert (declassify (!f));
  assert (declassify (!!t));
  assert (declassify (-one == none));
  assert (declassify (- -one == one));
}
