/**
 * Bug fixed by
 * cce43571913939d4a7e15815a720169bb0352d6d
 */
void main () {
  bool t = false;
  do {
    t = true;
  } while (false);
  assert (t);
  assert (t);
}
