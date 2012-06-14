#ifndef SECREC_BLOCKS_H
#define SECREC_BLOCKS_H

#include <set>
#include <boost/intrusive/list.hpp>
#include <boost/utility.hpp>

#include "icodelist.h"
#include "CFG.h"

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

struct Edge {
    enum Label {
        None     = 0x00,
        Jump     = 0x01,
        True     = 0x02,
        False    = 0x04,
        Call     = 0x08,
        Ret      = 0x10,
        CallPass = 0x20,
        End      = 0x40
    };

    static inline bool isLocal (Label label) {
        return (label & (Jump | True | False | CallPass)) != 0;
    }

    static inline bool isGlobal (Label label) {
        return (label & (Call | Ret)) != 0;
    }

    static const Label begin = Jump;
    static const Label end = End;
    static inline Label next (Label label) {
        return static_cast<Label>(label << 1);
    }
};

/**
 * \brief Representation of intermediate code basic block.
 * \todo This class requires major refactoring.
 */
class Block : private ImopList
            , public auto_unlink_hook
            , public CFGNode<Block, Edge::Label>
            , boost::noncopyable
{
public: /* Types: */

    typedef CFGNode<Block, Edge::Label> CFGBase;
    typedef std::set<Block*> Set;
    using ImopList::iterator;
    using ImopList::const_iterator;
    using ImopList::reverse_iterator;
    using ImopList::const_reverse_iterator;

public: /* Methods: */

    explicit Block (unsigned long i, Procedure* proc)
        : m_proc (proc)
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

    void unlink() {
        CFGBase::unlink ();
        auto_unlink_hook::unlink();
    }

    bool hasIncomingJumps () const;

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

    Procedure*     const  m_proc;      ///< Pointer to containing procedure
    unsigned long  const  m_index;     ///< Index of block
    bool                  m_reachable; ///< If block is reachable
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

private:

    void assignToBlocks (ICodeList& imops);
    void propagate ();
};

} // namespace SecreC

std::ostream &operator<<(std::ostream& out, const SecreC::Program& proc);

#endif // BLOCKS_H
