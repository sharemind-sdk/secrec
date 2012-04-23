
void main () {
  string sa = "hello, world!";
  assert (sa == sa);
  sa = sa + " " + sa;
  assert (sa == sa);
  assert (sa != "hi");

  assert ("" == "");
  assert ("" <= "");
  assert (! ("" <  ""));
  assert ("" >= "");
  assert (! ("" > ""));
  assert ("" != "a");
  assert ("a" != "");
  assert (! ("" != ""));
  assert ("abcd" < "b");

  return;
}
