
bool [[1]] sieve (int n) {
  bool [[1]] s [n];
  s[2:n] = true;
  int i; int j;
  for (i = 2; i*i < n; i += 1)
    if (s[i])
      for (j = 2*i; j < n; j += i)
        s [j] = false;
  return s;
}

void main () {
  bool [[1]] is_prime = sieve (100);
  assert (is_prime[2]);
  assert (is_prime[3]);
  assert (is_prime[5]);
  assert (is_prime[7]);
  assert (!is_prime[25]);
  assert (is_prime[97]);
}
