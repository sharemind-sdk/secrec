template <type A, type B> struct TestTemplate {}
template <type A, type B> void f(A a, B b) { TestTemplate<A, B> test; }

void main() {
  assert((true, false) == false);

  assert((true, false) || true);
  assert((false, true) || false);
  assert(true || (true, false));
  assert(false || (false, true));
  assert((true, false) || (false, true));
  assert((false, true) || (true, false));
  assert((false, true) && (false, true));
  assert(!(true, false));
  assert(false, true);
  assert((false, true));

  {
    string accum;
    uint acount = 5, j = 123, k = 0, c = 0;
    for (uint i = acount
         , j = acount + 1;
         ++k, i != 0;
         --i
         , --j)
    {
      accum += "i" + tostring(i) + "j" + tostring(j) + "k" + tostring(k)
               + "c" + tostring(c) + " ";
      ++c;
    }
    accum += "j" + tostring(j) + "k" + tostring(k) + "c" + tostring(c);
    assert(accum == "i5j6k1c0 i4j5k2c1 i3j4k3c2 i2j3k4c3 i1j2k5c4 j123k6c5");
  }
  {
    string accum;
    uint acount = 5, j = 123, i = 321, k = 0, c = 0;
    for (i = acount
         , j = acount + 1;
         ++k, i != 0;
         --i
         , --j)
    {
      accum += "i" + tostring(i) + "j" + tostring(j) + "k" + tostring(k)
               + "c" + tostring(c) + " ";
      ++c;
    }
    accum += "i" + tostring(i) + "j" + tostring(j) + "k" + tostring(k)
             + "c" + tostring(c);
    assert(accum == "i5j6k1c0 i4j5k2c1 i3j4k3c2 i2j3k4c3 i1j2k5c4 i0j1k6c5");
  }

  {
    string a = "duh";
    assert(
      ((a = "He", (a += "ll", a += "o,")), ((a += " Wo", a += "rld"), a += "!"))
      == "Hello, World!"
    );
  }
}
