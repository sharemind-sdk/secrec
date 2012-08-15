uint [[1]] nat (uint n) {
  uint [[1]] out (n) = 1;
  if (n == 1) return out;
  uint [[1]] rec = nat (n - 1);
  assert (size(rec) == (n - 1));
  out[1:n] += rec;
  return out;
}

void main () {
  uint [[1]] nats = nat (5 :: uint);
  assert (size(nats) == 5);
  for (uint i = 0; i < 5; ++ i) {
    assert (nats[i] == i + 1);
  }
}
