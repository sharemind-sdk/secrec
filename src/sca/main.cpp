#include <cassert>
#include <iostream>
#include <libscc/blocks.h>
#include <libscc/intermediate.h>
#include <libscc/parser.h>
#include <libscc/dataflowanalysis.h>
#include <libscc/treenode.h>

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
            return 1;
        }
    }
    fflush(stdout);
    fflush(stderr);

    if (parseResult == 0) {
        assert(parseTree != 0);
        cout << parseTree->toString() << endl << endl;

        SecreC::ICode icode;
        icode.init(parseTree);
        if (icode.status() == SecreC::ICode::OK) {
            SecreC::DataFlowAnalysis rd(icode);
            rd.run();
            cout << icode.blocks().toString(&rd)
                 << icode.compileLog();
        } else {
            cerr << "Error generating valid intermediate code." << endl
                 << icode.compileLog();
        }
    }
    delete parseTree;

    return parseResult;
}
