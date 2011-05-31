int [[1]] nat (int n) {
  int [[1]] out (n) = 1;
  if (n == 1) return out;
  int [[1]] rec = nat (n - 1);
  assert (size(rec) == (n - 1));
  out[1:n] += rec;
  return out;
}

void main () {
  int [[1]] nats = nat(5);
  int i;
  assert (size(nats) == 5);
  for (i = 0; i < 5; i += 1) {
    assert (nats[i] == i + 1);
  }
}
