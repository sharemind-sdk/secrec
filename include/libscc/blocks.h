#ifndef SECREC_BLOCKS_H
#define SECREC_BLOCKS_H

#include <list>
#include <map>
#include <set>
#include <boost/intrusive/list.hpp>
#include <boost/utility.hpp>

#include "icodelist.h"

namespace SecreC {

class Program;
class Procedure;
class Block;

typedef boost::intrusive::list_base_hook<
            boost::intrusive::link_mode<
                boost::intrusive::auto_unlink> > auto_unlink_hook;

/*******************************************************************************
  Block
*******************************************************************************/

/**
 * \brief Representation of intermediate code basic block.
 * \a Block doesn't own \a Imop and does not destroy them. \a ICodeList
 * is responsible of destroying all instructions.
 * \todo This class requires major refactoring.
 */
class Block : private ImopList, public auto_unlink_hook, boost::noncopyable {
public: /* Types: */

    typedef std::set<Block*> Set;
    using ImopList::iterator;
    using ImopList::const_iterator;
    using ImopList::reverse_iterator;
    using ImopList::const_reverse_iterator;

public: /* Methods: */

    explicit Block (unsigned long i, Procedure* proc)
        : m_callPassTo (0)
        , m_callPassFrom (0)
        , m_proc (proc)
        , m_index (i)
        , m_reachable (false)
    { }

    ~Block ();

    using ImopList::empty;
    using ImopList::begin;
    using ImopList::end;
    using ImopList::rbegin;
    using ImopList::rend;
    using ImopList::front;
    using ImopList::back;
    using ImopList::push_back;
    using ImopList::s_iterator_to;
    using ImopList::insert;

    /// \brief unlink block from CFG
    void unlink ();

    void addUser (Block* block) { m_users.insert (block); }

    // Predecessors:
    void addPred (Block* block) { m_predecessors.insert (block); }
    void addPredCondFalse (Block* block) { m_predecessorsCondFalse.insert (block); }
    void addPredCondTrue (Block* block) { m_predecessorsCondTrue.insert (block); }
    void addPredCall (Block* block) { m_predecessorsCall.insert (block); }
    void addPredRet (Block* block) { m_predecessorsRet.insert (block); }
    void setCallPassTo (Block* block) { m_callPassTo = block; }
    const std::set<Block*>& pred () const { return m_predecessors; }
    const std::set<Block*>& predCondFalse () const { return m_predecessorsCondFalse; }
    const std::set<Block*>& predCondTrue () const { return m_predecessorsCondTrue; }
    const std::set<Block*>& predCall () const { return m_predecessorsCall; }
    const std::set<Block*>& predRet () const { return m_predecessorsRet; }
    Block* callPassFrom () const { return m_callPassFrom; }

    // Successors:
    void addSucc (Block* block) { m_successors.insert (block); }
    void addSuccCondFalse (Block* block) { m_successorsCondFalse.insert (block); }
    void addSuccCondTrue (Block* block) { m_successorsCondTrue.insert (block); }
    void addSuccCall (Block* block) { m_successorsCall.insert (block); }
    void addSuccRet (Block* block) { m_successorsRet.insert (block); }
    void setCallPassFrom (Block* block) { m_callPassFrom = block; }
    const std::set<Block*>& succ () const { return m_successors; }
    const std::set<Block*>& succCondFalse () const { return m_successorsCondFalse; }
    const std::set<Block*>& succCondTrue () const { return m_successorsCondTrue; }
    const std::set<Block*>& succCall () const { return m_successorsCall; }
    const std::set<Block*>& succRet () const { return m_successorsRet; }
    Block* callPassTo () const { return m_callPassTo; }

    void setReachable () { m_reachable = true; }
    bool reachable () const { return m_reachable; }
    unsigned long index () const { return m_index; }

    void getIncoming (std::set<Block*>& list) const;
    void getOutgoing (std::set<Block*>& list) const;

    Procedure* proc () const { return m_proc; }

    bool isProgramExit () const;
    bool isProgramEntry () const;
    bool isExit () const;
    bool isEntry () const;

private: /* Fields: */

    std::set<Block*>        m_predecessors;
    std::set<Block*>        m_predecessorsCondFalse;
    std::set<Block*>        m_predecessorsCondTrue;
    std::set<Block*>        m_predecessorsCall;
    std::set<Block*>        m_predecessorsRet;
    std::set<Block*>        m_successors;
    std::set<Block*>        m_successorsCondFalse;
    std::set<Block*>        m_successorsCondTrue;
    std::set<Block*>        m_successorsCall;
    std::set<Block*>        m_successorsRet;
    std::set<Block*>        m_users;
    Block*                  m_callPassTo;
    Block*                  m_callPassFrom;
    Procedure* const        m_proc;                      ///< Pointer to containing procedure
    const unsigned long     m_index;                     ///< Index of block
    bool                    m_reachable;                 ///< If block is reachable
};

typedef boost::intrusive::list<Block, boost::intrusive::constant_time_size<false> > BlockList;

inline Block::iterator blockIterator (Imop& imop) {
    return Block::s_iterator_to (imop);
}

inline Block::const_iterator blockIterator (const Imop& imop) {
    return Block::s_iterator_to (imop);
}

/*******************************************************************************
  Procedure
*******************************************************************************/

class Procedure : private BlockList, public auto_unlink_hook, boost::noncopyable {
public: /* Types: */

    using BlockList::iterator;
    using BlockList::const_iterator;

public: /* Methods: */

    explicit Procedure (const SymbolProcedure* name)
        : m_name (name)
    { }

    ~Procedure ();

    const Block* entry () const {
        assert (!empty ());
        return &front ();
    }

    Block* entry () {
        assert (!empty ());
        return &front ();
    }

    const SymbolProcedure* name () const { return m_name; }
    const std::set<Block*>& callFrom () const { return m_callFrom; }
    const std::set<Block*>& returnTo () const { return m_returnTo; }
    const std::set<Block*>& exitBlocks () const { return m_exits; }

    void addCallFrom (Block& block) { m_callFrom.insert (&block); }
    void addReturnTo (Block& block) { m_returnTo.insert (&block); }
    void addExit (Block& block) { m_exits.insert (&block); }

    using BlockList::begin;
    using BlockList::end;
    using BlockList::front;
    using BlockList::back;
    using BlockList::empty;
    using BlockList::s_iterator_to;
    using BlockList::push_back;

private: /* Fields: */

    std::set<Block*>              m_exits;
    std::set<Block*>              m_callFrom;
    std::set<Block*>              m_returnTo;
    const SymbolProcedure* const  m_name;
};

typedef boost::intrusive::list<Procedure, boost::intrusive::constant_time_size<false> > ProcedureList;

/// Blocks can be converted to procedure iterator
inline Procedure::iterator procIterator (Block& block) {
    return BlockList::s_iterator_to (block);
}

inline Procedure::const_iterator procIterator (const Block& block) {
    return  BlockList::s_iterator_to (block);
}

/*******************************************************************************
  Program
*******************************************************************************/

class Program : private ProcedureList, boost::noncopyable {
public: /* Types: */

    using ProcedureList::iterator;
    using ProcedureList::const_iterator;

public: /* Methods: */

    Program ();
    ~Program ();

    void init (ICodeList& code);

    std::string toString () const;
    void toDotty (std::ostream& os) const;

    using ProcedureList::begin;
    using ProcedureList::end;
    using ProcedureList::empty;

    const Block* entryBlock () const;
    const Block* exitBlock () const;

protected:

    void assignToBlocks (ICodeList& imops);
    void propagate ();
};

} // namespace SecreC

std::ostream &operator<<(std::ostream& out, const SecreC::Program& proc);

#endif // BLOCKS_H
