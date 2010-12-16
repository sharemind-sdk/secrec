
void main () {
  string sa = "hello, world!";
  assert (sa == sa);
  sa = sa + " " + sa;
  assert (sa == sa);
  assert (sa != "hi");
  return;
}
