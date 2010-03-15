#ifndef TREENODEEXPR_H
#define TREENODEEXPR_H

#include "treenodecodeable.h"

namespace SecreC {

class TreeNodeExpr: public TreeNode {
    public: /* Types: */
        enum Flags { CONSTANT = 0x01, PARENTHESIS = 0x02 };

    public: /* Methods: */
        explicit TreeNodeExpr(Type type, const YYLTYPE &loc)
            : TreeNode(type, loc), m_result(0), m_resultType(0),
              m_firstImop(0) {}
        virtual ~TreeNodeExpr() {
            if (m_resultType != 0) delete *m_resultType;
            delete m_resultType;
        }

        virtual ICode::Status calculateResultType(SymbolTable &st,
                                                  std::ostream &es) = 0;
        virtual ICode::Status generateCode(ICode::CodeList &code,
                                           SymbolTable &st,
                                           std::ostream &es,
                                           SymbolWithValue *result = 0) = 0;
        virtual ICode::Status generateBoolCode(ICode::CodeList &code,
                                               SymbolTable &st,
                                               std::ostream &es)
        {
            assert(false);
        }

        inline SymbolWithValue *result() const {
            assert(m_result != 0);
            return m_result;
        };
        inline SecreC::Type *resultType() const {
            assert(m_resultType != 0);
            return *m_resultType;
        }
        inline const std::vector<Imop*> &falseList() const {
            return m_falseList;
        }
        inline const std::vector<Imop*> &trueList() const {
            return m_trueList;
        }
        inline const std::vector<Imop*> &nextList() const {
            return m_nextList;
        }
        inline Imop *firstImop() const {
            return m_firstImop;
        }
        void patchTrueList(Symbol *dest);
        void patchFalseList(Symbol *dest);
        void patchNextList(Symbol *dest);

    protected:
        inline SymbolWithValue *&result() {
            return m_result;
        };
        inline SecreC::Type **&resultType() {
            return m_resultType;
        }
        inline std::vector<Imop*> &falseList() {
            return m_falseList;
        }
        inline std::vector<Imop*> &trueList() {
            return m_trueList;
        }
        inline std::vector<Imop*> &nextList() {
            return m_nextList;
        }
        inline Imop *&firstImop() {
            return m_firstImop;
        }

    private: /* Fields: */
        SymbolWithValue    *m_result;
        SecreC::Type      **m_resultType;
        std::vector<Imop*>  m_falseList;
        std::vector<Imop*>  m_trueList;
        std::vector<Imop*>  m_nextList;
        Imop               *m_firstImop;

        /// \todo Add flags.
};

} // namespace SecreC

#endif // TREENODEEXPR_H
