kind a3p;
domain private a3p;

int one () { return 1; }

private bool ft () { return true; }

void ret () {
  if (true) return;
  assert (false);
}

void main() {
  ret ();
  assert (one() == 1);
  assert (declassify (ft() == true));
}
