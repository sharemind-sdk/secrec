
int add (int x, int y)
{
  return x + y;
}

void main ()
{
  int a = 1;
  int b = 1;
  int c = add (a, b);
  assert (c == 2);
}
