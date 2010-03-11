#ifndef TREENODEEXPRTERNARY_H
#define TREENODEEXPRTERNARY_H

#include "treenode.h"


class TreeNodeExprTernary: public TreeNodeExpr {
    public: /* Methods: */
        explicit TreeNodeExprTernary(const YYLTYPE &loc)
            : TreeNodeExpr(NODE_EXPR_TERNIF, loc), m_result(0), m_resultType(0) {}
        virtual ~TreeNodeExprTernary() {
            if (m_resultType != 0) delete *m_resultType;
            delete m_resultType;
        }

        virtual const Symbol *result() const {
            assert(m_result != 0);
            return m_result;
        }
        virtual inline const SecreC::Type *resultType() const {
            assert(m_resultType != 0);
            return *m_resultType;
        }
        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es);
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es);

    private: /* Fields: */
        const Symbol *m_result;
        const SecreC::Type **m_resultType;
};

#endif // TREENODEEXPRTERNARY_H
