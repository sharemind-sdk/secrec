
bool [[1]] sieve (int n) {
  bool [[1]] s (n) = true;
  int i; int j;
  s[0:2] = false;
  for (i = 2; i*i < n; i += 1)
    if (s[i])
      for (j = 2*i; j < n; j += i)
        s [j] = false;
  return s;
}

int count_true (bool [[1]] arr) {
  int i; int count = 0;
  for (i = 0; i < size(arr); i += 1)
    if (arr[i])
      count += 1;
  return count;
}

void main () {
  bool [[1]] is_prime = sieve (100);
  assert (is_prime[2]);
  assert (is_prime[3]);
  assert (is_prime[5]);
  assert (is_prime[7]);
  assert (!is_prime[25]);
  assert (is_prime[97]);
  assert (count_true(is_prime) == 25);
}
