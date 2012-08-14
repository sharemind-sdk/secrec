
bool [[1]] sieve (uint n) {
  bool [[1]] s (n) = true;
  s[0:2] = false;
  for (uint i = 2; i*i < n; i += 1)
    if (s[i])
      for (uint j = 2*i; j < n; j += i)
        s[j] = false;
  return s;
}

uint count_true (bool [[1]] arr) {
  uint count = 0;
  for (uint i = 0; i < size (arr); ++ i)
      count += (uint) arr[i];
  return count;
}

void main () {
  bool [[1]] is_prime = sieve (100 :: uint);
  assert (is_prime[2]);
  assert (is_prime[3]);
  assert (is_prime[5]);
  assert (is_prime[7]);
  assert (!is_prime[25]);
  assert (is_prime[97]);
  assert (count_true(is_prime) == 25);
}
