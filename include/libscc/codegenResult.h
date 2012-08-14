#ifndef SECREC_CODE_GEN_RESULT_H
#define SECREC_CODE_GEN_RESULT_H

#include "intermediate.h"

#include <list>

namespace {

inline void patchList(std::vector<SecreC::Imop*> &list, SecreC::SymbolLabel *dest) {
    typedef std::vector<SecreC::Imop*>::const_iterator IVCI;
    for (IVCI it(list.begin()); it != list.end(); it++) {
        (*it)->setJumpDest(dest);
    }
    list.clear();
}

}

namespace SecreC {

class ICodeList;
class SymbolTable;
class CompileLog;

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

    enum Status { OK, ERROR_FATAL, ERROR_CONTINUE };

public: /* Methods: */

    inline CGResult(const Status s = OK)
        : m_result (0)
        , m_firstImop (0)
        , m_status (s)
    { }

    Imop* firstImop (void) const {
        return m_firstImop;
    }

    void setNextList (const std::vector<Imop*>& list) {
        m_nextList = list;
    }

    const std::vector<Imop*>& nextList (void) const {
        return m_nextList;
    }

    void addToNextList (Imop* imop) {
        m_nextList.push_back (imop);
    }

    void addToNextList (const std::vector<Imop* >& nl) {
        m_nextList.insert (m_nextList.end (), nl.begin (), nl.end ());
    }

    void setResult (Symbol* sym) {
        m_result = sym;
    }

    void setFirstImop (Imop* imop) {
        m_firstImop = imop;
    }

    void patchNextList (SymbolLabel* dest) {
        patchList (m_nextList, dest);
    }

    void patchFirstImop (Imop* imop) {
        if (m_firstImop == 0) {
            m_firstImop = imop;
        }
    }

    Symbol* symbol () const {
        return m_result;
    }

    inline bool isOk (void) const {
        return m_status == OK;
    }

    inline bool isNotOk (void) const {
        return m_status != OK;
    }

    Status status () const {
        return m_status;
    }

    void setStatus (Status status) {
        m_status = status;
    }

private: /* Fields: */
    std::vector<Imop* >   m_nextList;     ///< unpatched jumps to next imop
    Symbol*               m_result;       ///< symbol the result is stored in
    Imop*                 m_firstImop;    ///< pointer to the first instruction
    Status                m_status;       ///< status of the code generation
};

/*******************************************************************************
  CGBranchResult
*******************************************************************************/

/// Code generation result which also tracks true and false lists.
class CGBranchResult : public CGResult {
public:

    inline CGBranchResult (Status status = OK)
        : CGResult (status)
    { }

    inline CGBranchResult (const CGResult& result)
        : CGResult (result)
    { }

    void swapTrueFalse () {
        std::swap (m_trueList, m_falseList);
    }

    const std::vector<Imop*>& trueList () const {
        return m_trueList;
    }

    const std::vector<Imop*>& falseList () const {
        return m_falseList;
    }

    void setTrueList (const std::vector<Imop*>& tl) {
        m_trueList = tl;
    }

    void setFalseList (const std::vector<Imop*>& fl) {
        m_falseList = fl;
    }

    void addToTrueList (Imop* imop) {
        m_trueList.push_back (imop);
    }

    void addToFalseList (const std::vector<Imop*>& fl) {
        m_falseList.insert (m_falseList.end (), fl.begin (), fl.end ());
    }

    void addToTrueList (const std::vector<Imop*>& tl) {
        m_trueList.insert (m_trueList.end (), tl.begin (), tl.end ());
    }

    void addToFalseList (Imop* imop) {
        m_falseList.push_back (imop);
    }

    void patchTrueList (SymbolLabel* dest) {
        patchList (m_trueList, dest);
    }

    void patchFalseList (SymbolLabel* dest) {
        patchList (m_falseList, dest);
    }

private:
    std::vector<Imop* > m_trueList;    ///< unpatched jumps in case conditional is true
    std::vector<Imop* > m_falseList;   ///< unpatched jumps in case conditional is false
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

    void addToBreakList (const std::vector<Imop*>& bl) {
        m_breakList.insert (m_breakList.end (), bl.begin (), bl.end ());
    }

    void addToContinueList (const std::vector<Imop*>& cl) {
        m_continueList.insert (m_continueList.end (), cl.begin (), cl.end ());
    }

    void patchBreakList (SymbolLabel* dest) {
        patchList (m_breakList, dest);
    }

    void patchContinueList (SymbolLabel* dest) {
        patchList (m_continueList, dest);
    }

    const std::vector<Imop*>& breakList () const {
        return m_breakList;
    }

    void clearBreakList () { m_breakList.clear (); }

    const std::vector<Imop*>& continueList () const {
        return m_continueList;
    }

    inline int flags () const {
        return m_resultFlags;
    }

    inline bool mayFallThrough () const {
        return (m_resultFlags & FALLTHRU) != 0;
    }

    inline void setFlags (int flags) {
        m_resultFlags = flags;
    }

private:
    std::vector<Imop*>  m_continueList;  ///< Unpatched continue jumps.
    std::vector<Imop*>  m_breakList;     ///< Unpatched break jumps.
    int                 m_resultFlags;   ///< Flag to track the possibilities that control flow may take.
};

}

#endif
