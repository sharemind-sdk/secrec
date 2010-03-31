#include <cassert>
#include <iostream>
#include <libscc/secrec/parser.h>
#include <libscc/secrec/treenode.h>
#include <libscc/intermediate.h>

using namespace std;

int main() {
    SecreC::TreeNodeProgram *parseTree = 0;

    int parseResult = sccparse(&parseTree);
    fflush(stdout);
    fflush(stderr);

    if (parseResult == 0) {
        assert(parseTree != 0);
        cerr << parseTree->toString() << endl << endl;
        cout << SecreC::ICode(parseTree);
    }
    delete parseTree;

    return parseResult;
}
