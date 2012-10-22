#ifndef SECREC_BLOCKS_H
#define SECREC_BLOCKS_H

#include <set>
#include <boost/intrusive/list.hpp>
#include <boost/utility.hpp>

#include "icodelist.h"
#include "CFG.h"

#define FOREACH_BLOCK(IT,pr) \
    for (Program::const_iterator pit = pr.begin (); pit != pr.end (); ++ pit)\
        for (Procedure::const_iterator IT = pit->begin (); IT != pit->end (); ++ IT)

namespace SecreC {

class Program;
class Procedure;
class Block;

typedef boost::intrusive::list_base_hook<
            boost::intrusive::link_mode<
                boost::intrusive::auto_unlink> > auto_unlink_hook;

/*******************************************************************************
  Edge
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
        assert ((label & (label - 1)) == 0 &&
                "Iterating non-power-of-two label!");
        return static_cast<Label>(label << 1);
    }
};

/*******************************************************************************
  Block
*******************************************************************************/

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

    Block ()
        : m_proc (0)
        , m_dfn (0)
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
    using ImopList::s_iterator_to;
    using ImopList::insert;

    void push_back (Imop& imop) {
        ImopList::push_back (imop);
        imop.setBlock (this);
    }

    void unlink () {
        CFGBase::unlink ();
        auto_unlink_hook::unlink();
        m_proc = 0;
        m_dfn = 0;
        m_reachable = false;
    }

    bool reachable () const { return m_reachable; }
    size_t index () const { return m_dfn; }
    size_t dfn () const { return m_dfn; }
    Procedure* proc () const { return m_proc; }
    bool hasIncomingJumps () const;
    bool isProgramExit () const;
    bool isProgramEntry () const;
    bool isExit () const;
    bool isEntry () const;

    void setReachable () { m_reachable = true; }
    void setDfn (size_t n) { m_dfn = n; }
    void setProc (Procedure* proc) { m_proc = proc; }

private: /* Fields: */
    Procedure* m_proc;      ///< Pointer to containting procedure
    size_t     m_dfn;       ///< Depth-first number of the block
    bool       m_reachable; ///< If block is reachable
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

class Procedure : private BlockList
                , public auto_unlink_hook
                , boost::noncopyable
{
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

    void push_back (Block& block) {
        BlockList::push_back (block);
        block.setProc (this);
    }

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

    std::ostream& print (std::ostream& os) const;
    void toDotty (std::ostream& os) const;

    using ProcedureList::begin;
    using ProcedureList::end;
    using ProcedureList::empty;

    const Block* entryBlock () const;
    const Block* exitBlock () const;

private:

    void assignToBlocks (ICodeList& imops);
    void propagate ();
    void numberBlocks ();
};

inline std::ostream &operator<<(std::ostream& out, const Program& proc) {
    return proc.print(out);
}

} // namespace SecreC

#endif // BLOCKS_H
