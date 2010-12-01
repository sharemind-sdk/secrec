int get_int (int n) { return 42; }
int get_int (bool b) { return 1; }
void main() {
  bool t = true;
  int one = 1;
  assert (get_int(t) == 1);
  assert (get_int(one) == 42);
}
