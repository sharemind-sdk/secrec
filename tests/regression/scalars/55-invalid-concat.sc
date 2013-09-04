void main() {
  uint32[[1]] foo1 = {1, 2};
  uint32[[1]] foo2 = {3, 4};
  foo1 = cat(foo1, foo2, 1);
}
