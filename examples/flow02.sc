/*

  Data flow analyzer example #2.
  ------------------------------

  This is an example with loops.

  NOTE! Since the analyzer does not yet support detection of constant
  conditionals, it expects that conditionals can have any value. Hence it
  presumes that in the control flow graph (CFG), both paths could be taken.

*/

void main() {
    // Basic block is here
    public int a;
    while (1 == 1) {
        // Basic block is here
        if (2 == 2) {
            // Basic block is here
            1 + 1;
        } else {
            // Basic block is here
            2 + 3;
        }
        // Basic block is here
        a = 2;
        if (3 == 3) {
            // Basic block is here
            a = 3;
        } else {
            // Basic block is here
            3 + 3;
        }
        // Basic block is here
        4 + 4;
    }
    // Basic block is here
}

