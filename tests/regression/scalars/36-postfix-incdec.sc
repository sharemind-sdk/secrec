
// basic parsing and semantic checks for postfix increment and decrement
void main () {
  int i;

  i = 0;
  i ++; 
  assert (i == 1);

  i = 0;
  assert (i ++ == 0);

  i = 0;
  i = i ++ + 1;
  assert (i == 1);

  i = 0;
  i = 1 + i ++;
  assert (i == 1);

  i = 1;
  i --; 
  assert (i == 0);

  i = 1;
  assert (i -- == 1);

  i = 0;
  i = i -- + 1;
  assert (i == 1);

  i = 0;
  i = 1 + i --;
  assert (i == 1);
}
