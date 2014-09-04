#ifndef SECREC_CODE_GEN_RESULT_H
#define SECREC_CODE_GEN_RESULT_H

#include <cstddef>
#include <vector>

namespace SecreC {

class ICodeList;
class SymbolTable;
class CompileLog;
class Imop;
class Symbol;
class SymbolLabel;

typedef std::vector<Imop*> PatchList;

/*******************************************************************************
  CGResult
*******************************************************************************/

/**
 * \class CGResult
 * \brief Code generation result for expressions.
 * It tracks:
 * \li symbol for stored value
 * \li code generation status
 * \li unpatched instructions that jumps to next instruction
 * \li first instruction generated
 * \li pointer to currently processed tree node
 * Also serves as base class for other kinds of code gen results.
 */
class CGResult {
public: /* Types: */

    // lower bit denotes if the CG should halt
    // higher bit denotes if the CG has failed
    // composition of statuses is bitwise or
    enum Status {
        OK             = 0x0,
        ERROR_CONTINUE = 0x2,
        ERROR_FATAL    = 0x3
    };

public: /* Methods: */

    inline CGResult(const Status s = OK)
        : m_result (NULL)
        , m_firstImop (NULL)
        , m_status (s)
    { }

    Imop* firstImop (void) const {
        return m_firstImop;
    }

    void setNextList (const PatchList& list) {
        m_nextList = list;
    }

    const PatchList& nextList (void) const {
        return m_nextList;
    }

    void addToNextList (Imop* imop) {
        m_nextList.push_back (imop);
    }

    void addToNextList (const PatchList& nl) {
        m_nextList.insert (m_nextList.end (), nl.begin (), nl.end ());
    }

    void setResult (Symbol* sym) {
        m_result = sym;
    }

    void setFirstImop (Imop* imop) {
        m_firstImop = imop;
    }

    void patchNextList (SymbolLabel* dest);

    void patchFirstImop (Imop* imop) {
        if (m_firstImop == NULL) {
            m_firstImop = imop;
        }
    }

    Symbol* symbol () const {
        return m_result;
    }

    inline bool isOk () const {
        return m_status == OK;
    }

    inline bool isNotOk () const {
        return m_status != OK;
    }

    inline bool isFatal () const {
        return m_status == ERROR_FATAL;
    }

    Status status () const {
        return m_status;
    }

    void setStatus (Status status) {
        m_status = status;
    }

    CGResult& operator |= (Status status) {
        m_status = static_cast<Status>(m_status | status);
        return *this;
    }

    const std::vector<Symbol*>& symbols () const {
        return m_symbols;
    }

    void markSymbol (Symbol* sym) {
        m_symbols.push_back (sym);
    }

    void markSymbol (const std::vector<Symbol*>& syms) {
        markSymbol (syms.begin (), syms.end ());
    }

    template <typename Iter>
    void markSymbol (Iter begin, Iter end) {
        m_symbols.insert (m_symbols.end (), begin, end);
    }

private: /* Fields: */
    PatchList            m_nextList;  ///< unpatched jumps to next imop
    Symbol*              m_result;    ///< symbol the result is stored in
    Imop*                m_firstImop; ///< pointer to the first instruction
    Status               m_status;    ///< status of the code generation
    std::vector<Symbol*> m_symbols;   ///< generated symbols
};

/*******************************************************************************
  CGBranchResult
*******************************************************************************/

/// Code generation result which also tracks true and false lists.
class CGBranchResult : public CGResult {
public: /* Methods: */

    inline CGBranchResult (Status status = OK)
        : CGResult (status)
    { }

    inline CGBranchResult (const CGResult& result)
        : CGResult (result)
    { }

    void swapTrueFalse () {
        std::swap (m_trueList, m_falseList);
    }

    const PatchList& trueList () const {
        return m_trueList;
    }

    const PatchList& falseList () const {
        return m_falseList;
    }

    void setTrueList (const PatchList& tl) {
        m_trueList = tl;
    }

    void setFalseList (const PatchList& fl) {
        m_falseList = fl;
    }

    void addToTrueList (Imop* imop) {
        m_trueList.push_back (imop);
    }

    void addToFalseList (const PatchList& fl) {
        m_falseList.insert (m_falseList.end (), fl.begin (), fl.end ());
    }

    void addToTrueList (const PatchList& tl) {
        m_trueList.insert (m_trueList.end (), tl.begin (), tl.end ());
    }

    void addToFalseList (Imop* imop) {
        m_falseList.push_back (imop);
    }

    void patchTrueList (SymbolLabel* dest);
    void patchFalseList (SymbolLabel* dest);

private: /* Fields: */
    PatchList m_trueList;    ///< unpatched jumps in case conditional is true
    PatchList m_falseList;   ///< unpatched jumps in case conditional is false
};

/*******************************************************************************
  CGStmtResult
*******************************************************************************/

/// Code generation result which also tracks break and continue lists.
class CGStmtResult : public CGResult {
public: /* Types: */
    enum ResultClass {
        FALLTHRU = 0x01,   ///< may fall through
        RETURN   = 0x02,   ///< may return
        BREAK    = 0x04,   ///< may break
        CONTINUE = 0x08,   ///< may continue
        MASK     = 0x0f
    };

public:
    inline CGStmtResult (Status status = OK)
        : CGResult (status)
        , m_resultFlags (FALLTHRU)
    { }

    inline CGStmtResult (const CGResult& result)
        : CGResult (result)
        , m_resultFlags (FALLTHRU)
    { }

    void addToBreakList (Imop* imop) {
        m_breakList.push_back (imop);
    }

    void addToContinueList (Imop* imop) {
        m_continueList.push_back (imop);
    }

    void addToBreakList (const PatchList& bl) {
        m_breakList.insert (m_breakList.end (), bl.begin (), bl.end ());
    }

    void addToContinueList (const PatchList& cl) {
        m_continueList.insert (m_continueList.end (), cl.begin (), cl.end ());
    }

    void patchBreakList (SymbolLabel* dest);
    void patchContinueList (SymbolLabel* dest);

    const PatchList& breakList () const {
        return m_breakList;
    }

    void clearBreakList () { m_breakList.clear (); }

    const PatchList& continueList () const {
        return m_continueList;
    }

    inline int flags () const {
        return m_resultFlags;
    }

    inline bool mayFallThrough () const {
        return (m_resultFlags & FALLTHRU) != 0x0;
    }

    inline void setFlags (int flags) {
        m_resultFlags = flags;
    }

private: /* Fields: */
    PatchList  m_continueList;  ///< Unpatched continue jumps.
    PatchList  m_breakList;     ///< Unpatched break jumps.
    int        m_resultFlags;   ///< Flag to track the possibilities that control flow may take.
};

} // namespace SecreC

#endif
