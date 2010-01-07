#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libscc/parser.h>
#include <libscc/treenode.h>
#include <libscc/tnsymbols.h>

int main() {
    struct TreeNode *parseTree = 0;

    int parseResult = sccparse(&parseTree);
    fflush(stdout);
    fflush(stderr);

    if (parseResult == 0) {
        assert(parseTree != 0);
        treenode_print(parseTree, stderr, 2);
        treenode_printXml(parseTree, stdout);
        fflush(stdout);
        fflush(stderr);
        struct TNSymbols *s = tnsymbols_init(parseTree);
        unsigned c = tnsymbols_unresolvedCount(s);
        printf("%u unresolved symbols found:\n", c);
        for (unsigned i = 0; i < c; i++) {
            const struct TreeNode *n = tnsymbols_unresolvedAt(s, i);
            printf("\t%s on line (%u,%u)-(%u,%u)\n",
                   treenode_value_string(n),
                   treenode_location(n)->first_line,
                   treenode_location(n)->first_column,
                   treenode_location(n)->last_line,
                   treenode_location(n)->last_column
                   );
        }
        tnsymbols_free(s);
    }
    if (parseTree != 0) treenode_free(parseTree);

    return parseResult;
}
