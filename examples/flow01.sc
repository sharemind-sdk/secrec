/*
  Data flow analyzer example #1.
  ------------------------------

  Names of parameters of procedure f are consist of two letters. The first
  letter signifies how the variable is treated by the first if-statement in the
  procedure, the second letter signifies the same for the second if-statement.
  The letters themselves have the following meaning:

    *  n  - the variable is not redefined inside the if-statement
    *  f  - the variable is redefined in the first branch of the if-statement
    *  s  - the variable is redefined in the second branch of the if-statement
    *  b  - the variable is redefined in both branches of the if-statement

  NOTE! Since the analyzer does not yet support detection of constant
  conditionals, it expects that conditionals can have any value. Hence it
  presumes that in the control flow graph (CFG), both paths could be taken.

*/

void f(
   private int nn, private int nf, private int ns, private int nb,
   private int fn, private int ff, private int fs, private int fb,
   private int sn, private int sf, private int ss, private int sb,
   private int bn, private int bf, private int bs, private int bb
) {
    // Basic block is here
    if (1 == 1) {
        // Basic block is here
        fn = ff = fs = fb = bn = bf = bs = bb = 1;
    } else {
        // Basic block is here
        sn = sf = ss = sb = bn = bf = bs = bb = 2;
    }
    // Basic block is here
    if (2 == 2) {
        // Basic block is here
        nf = ff = sf = bf = nb = fb = sb = bb = 3;
    } else {
        // Basic block is here
        ns = fs = ss = bs = nb = fb = sb = bb = 4;
    }
    // Basic block is here
}

// main() is compulsory:
void main() {
  // Call f with some "random" parameters:
  f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
}
