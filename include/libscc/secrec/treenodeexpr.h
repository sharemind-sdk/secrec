#ifndef TREENODEEXPR_H
#define TREENODEEXPR_H

#include "treenodecodeable.h"

namespace SecreC {

class TreeNodeExpr: public TreeNodeCodeable {
    public: /* Types: */
        enum Flags { CONSTANT = 0x01, PARENTHESIS = 0x02 };

    public: /* Methods: */
        explicit TreeNodeExpr(Type type, const YYLTYPE &loc)
            : TreeNodeCodeable(type, loc) {}

        virtual const Symbol *result() const = 0;
        virtual const SecreC::Type *resultType() const = 0;
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es) = 0;

    private: /* Fields: */
        /// \todo Add flags.
};

} // namespace SecreC

#endif // TREENODEEXPR_H
