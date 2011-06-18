#ifndef CODEGENRESULT_H
#define CODEGENRESULT_H

#include "intermediate.h"

#include <list>

namespace {

void patchList(std::list<SecreC::Imop*> &list, SecreC::SymbolLabel *dest) {
    typedef std::list<SecreC::Imop*>::const_iterator IVCI;
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
 * Code generation result for expressions. It tracks:
 * \li symbol for stored value
 * \li code generation status
 * \li unpatched instructions that jumps to next instruction
 * \li first instruction generated
 * \li pointer to currently processed tree node
 * Also serves as base class for other kinds of code gen results.
 */
class CGResult {
public:

    inline explicit CGResult (ICode::Status s = ICode::OK)
        : m_result (0),
          m_firstImop (0),
          m_status (s)
    { }

    inline ~CGResult () { }

    Imop* firstImop (void) const {
        return m_firstImop;
    }

    void setNextList (const std::list<Imop*>& list) {
        m_nextList = list;
    }

    const std::list<Imop*>& nextList (void) const {
        return m_nextList;
    }

    void addToNextList (Imop* imop) {
        m_nextList.push_back (imop);
    }

    void addToNextList (const std::list<Imop* >& nl) {
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
        return m_status == ICode::OK;
    }

    inline bool isNotOk (void) const {
        return m_status != ICode::OK;
    }

    ICode::Status status () const {
        return m_status;
    }

    void setStatus (ICode::Status status) {
        m_status = status;
    }

private:
    std::list<Imop* >   m_nextList;   ///< unpatched jumps to next imop
    Symbol*             m_result;     ///< symbol the result is stored in
    Imop*               m_firstImop;  ///< pointer to the first instruction
    ICode::Status       m_status;     ///< status of the code generation
};

/*******************************************************************************
  CGBranchResult
*******************************************************************************/

/// may branch
class CGBranchResult : public CGResult {
public:

    inline explicit CGBranchResult (ICode::Status status = ICode::OK)
        : CGResult (status)
    { }

    inline CGBranchResult (const CGResult& result)
        : CGResult (result)
    { }

    inline ~CGBranchResult () { }

    void swapTrueFalse () {
        std::swap (m_trueList, m_falseList);
    }

    const std::list<Imop*>& trueList () const {
        return m_trueList;
    }

    const std::list<Imop*>& falseList () const {
        return m_falseList;
    }

    void setTrueList (const std::list<Imop*>& tl) {
        m_trueList = tl;
    }

    void setFalseList (const std::list<Imop*>& fl) {
        m_falseList = fl;
    }

    void addToTrueList (Imop* imop) {
        m_trueList.push_back (imop);
    }

    void addToFalseList (const std::list<Imop*>& fl) {
        m_falseList.insert (m_falseList.end (), fl.begin (), fl.end ());
    }

    void addToTrueList (const std::list<Imop*>& tl) {
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
    std::list<Imop* > m_trueList;    ///< unpatched jumps in case conditional is true
    std::list<Imop* > m_falseList;   ///< unpatched jumps in case conditional is false
};

/*******************************************************************************
  CGStmtResult
*******************************************************************************/

/// may break, continue or return
class CGStmtResult : public CGResult {
public: /* Types: */
    enum ResultClass {
        FALLTHRU = 0x01,
        RETURN   = 0x02,
        BREAK    = 0x04,
        CONTINUE = 0x08,
        MASK     = 0x0f
    };

public:
    inline explicit CGStmtResult (ICode::Status status = ICode::OK)
        : CGResult (status),
          m_resultFlags (FALLTHRU)
    { }

    inline CGStmtResult (const CGResult& result)
        : CGResult (result),
          m_resultFlags (FALLTHRU)
    { }

    inline ~CGStmtResult () { }

    void addToBreakList (Imop* imop) {
        m_breakList.push_back (imop);
    }

    void addToContinueList (Imop* imop) {
        m_continueList.push_back (imop);
    }

    void addToBreakList (const std::list<Imop*>& bl) {
        m_breakList.insert (m_breakList.end (), bl.begin (), bl.end ());
    }

    void addToContinueList (const std::list<Imop*>& cl) {
        m_continueList.insert (m_continueList.end (), cl.begin (), cl.end ());
    }

    void patchBreakList (SymbolLabel* dest) {
        patchList (m_breakList, dest);
    }

    void patchContinueList (SymbolLabel* dest) {
        patchList (m_continueList, dest);
    }

    const std::list<Imop*>& breakList () const {
        return m_breakList;
    }

    void clearBreakList () { m_breakList.clear (); }

    const std::list<Imop*>& continueList () const {
        return m_continueList;
    }

    inline int flags () const {
        return m_resultFlags;
    }

    inline void setFlags (int flags) {
        m_resultFlags = flags;
    }

private:
    std::list<Imop*> m_continueList;
    std::list<Imop*> m_breakList;
    int m_resultFlags;
};

}

#endif
