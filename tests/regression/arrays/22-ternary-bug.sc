// \todo causes "double free or corruption" on pop_frame
// it looks like bug in VM, but could easily be code gen
void main () {
  bool [[1]] t (5);
  int [[1]] t2 (5);
  assert (size(t ? t2 : t2) == 5);
}
