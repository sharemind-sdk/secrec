#include <cassert>
#include <iostream>
#include <libscc/secrec/parser.h>
#include <libscc/secrec/treenode.h>
#include <libscc/blocks.h>
#include <libscc/intermediate.h>

using namespace std;

int main(int argc, char *argv[]) {
    char *filename = 0;
    if (argc > 1) {
        filename = argv[1];
    }
    SecreC::TreeNodeProgram *parseTree = 0;

    int parseResult;
    if (filename == 0) {
        parseResult = sccparse(&parseTree);
    } else {
        FILE *f = fopen(filename, "r");
        if (f != NULL) {
            cerr << "Parsing file: " << filename << endl;
            parseResult = sccparse_file(f, &parseTree);
            fclose(f);
        } else {
            cerr << "Unable to open file: " << filename << endl;
        }
    }
    fflush(stdout);
    fflush(stderr);

    if (parseResult == 0) {
        assert(parseTree != 0);
        cerr << parseTree->toString() << endl << endl;

        SecreC::ICode icode(parseTree);
        cout << icode;
        if (icode.status() == SecreC::ICode::OK) {
            SecreC::Blocks bs(icode.code());
            if (bs.status() == SecreC::Blocks::OK) {
                cout << bs;
            } else {
                cout << "Error generating valid basic blocks." << endl;
            }
        }
    }
    delete parseTree;

    return parseResult;
}
