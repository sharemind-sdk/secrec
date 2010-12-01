/**
 * for loop with empty body, failed codegen
 * Fixed: e3ceae772ecb3dc24863758a412ec110bf48810b
 */
void main () {
  int i;
  for (i = 0; i < 10; i = i + 1);
  assert (i == 10);
}
