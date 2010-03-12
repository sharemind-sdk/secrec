#include <cassert>
#include <iostream>
#include <libscc/secrec/parser.h>
#include <libscc/secrec/treenodeprogram.h>
#include <libscc/secrec/treenodeidentifier.h>
#include <libscc/secrec/tnsymbols.h>
#include <libscc/intermediate.h>

using namespace std;

int main() {
    SecreC::TreeNodeProgram *parseTree = 0;

    int parseResult = sccparse(&parseTree);
    fflush(stdout);
    fflush(stderr);

    if (parseResult == 0) {
        assert(parseTree != 0);
        cerr << parseTree->toString() << endl;
        cout << parseTree->toXml() << endl;
        SecreC::TNSymbols *s = new SecreC::TNSymbols(parseTree);
        unsigned c = s->unresolved().size();
        cout << c << " unresolved symbols found:" << endl;
        for (unsigned i = 0; i < c; i++) {
            const SecreC::TreeNodeIdentifier *n = s->unresolved().at(i);
            cout << "\t" << n->value() << " on line ("
                         << n->location().first_line << ","
                         << n->location().first_column << ")-("
                         << n->location().last_line << ","
                         << n->location().last_column << ")" << endl;
        }
        delete s;
    }
    if (parseTree != 0) delete parseTree;

    return parseResult;
}
