/**
 * Bug fixed with
 * ef869a3fcce29d8ab267da5bcb4c4f70a1d7e91a
 */
void main () {
  bool t = true;
  assert ((t && t) == t);
}
