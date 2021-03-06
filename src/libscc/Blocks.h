/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#ifndef SECREC_BLOCKS_H
#define SECREC_BLOCKS_H

#include "CFG.h"
#include "ICodeList.h"

#include <boost/intrusive/list.hpp>
#include <set>

#define FOREACH_BLOCK(IT,pr) \
    for (Program::const_iterator pit = pr.begin (); pit != pr.end (); ++ pit)\
        for (Procedure::const_iterator IT = pit->begin (); IT != pit->end (); ++ IT)

namespace SecreC {

class Program;
class Procedure;
class Block;

using auto_unlink_hook =
    boost::intrusive::list_base_hook<
        boost::intrusive::link_mode<
            boost::intrusive::auto_unlink> > ;

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
{
public: /* Types: */

    using CFGBase = CFGNode<Block, Edge::Label>;
    using Set = std::set<Block*>;
    using ImopList::iterator;
    using ImopList::const_iterator;
    using ImopList::reverse_iterator;
    using ImopList::const_reverse_iterator;

public: /* Methods: */

    Block ()
        : m_proc (nullptr)
        , m_dfn (0)
        , m_reachable (false)
    { }

    Block(Block const &) = delete;
    Block & operator = (Block const &) = delete;
    ~Block ();

    using ImopList::back;
    using ImopList::begin;
    using ImopList::empty;
    using ImopList::end;
    using ImopList::erase;
    using ImopList::front;
    using ImopList::insert;
    using ImopList::rbegin;
    using ImopList::rend;
    using ImopList::s_iterator_to;

    void push_back (Imop& imop) {
        ImopList::push_back (imop);
        imop.setBlock (this);
    }

    void unlink () {
        CFGBase::unlink ();
        auto_unlink_hook::unlink();
        m_proc = nullptr;
        m_dfn = 0;
        m_reachable = false;
    }

    bool reachable () const { return m_reachable; }
    size_t index () const { return m_dfn; }
    size_t dfn () const { return m_dfn; }
    Procedure* proc () const { return m_proc; }
    bool hasIncomingJumps () const;
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

using BlockList = boost::intrusive::list<Block, boost::intrusive::constant_time_size<false>>;

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
{
public: /* Types: */

    using BlockList::iterator;
    using BlockList::const_iterator;

public: /* Methods: */

    explicit Procedure (const SymbolProcedure* name)
        : m_name (name)
    { }

    Procedure(Procedure const &) = delete;
    Procedure & operator = (Procedure const &) = delete;
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

    void removeCallFrom (Block& block) { m_callFrom.erase (&block); }
    void removeReturnTo (Block& block) { m_returnTo.erase (&block); }

    using BlockList::back;
    using BlockList::begin;
    using BlockList::empty;
    using BlockList::end;
    using BlockList::front;
    using BlockList::insert;
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

using ProcedureList = boost::intrusive::list<Procedure, boost::intrusive::constant_time_size<false>>;

/// Blocks can be converted to procedure iterator
inline Procedure::iterator procIterator (Block& block) {
    return BlockList::s_iterator_to (block);
}

inline Procedure::const_iterator procIterator (const Block& block) {
    return BlockList::s_iterator_to (block);
}

/*******************************************************************************
  Program
*******************************************************************************/

class Program : private ProcedureList {
public: /* Types: */

    using ProcedureList::iterator;
    using ProcedureList::const_iterator;

public: /* Methods: */

    Program ();
    Program(Program const &) = delete;
    Program const & operator = (Program const &) = delete;
    ~Program ();

    void init (ICodeList& code);

    std::ostream& print (std::ostream& os) const;
    void toDotty (std::ostream& os) const;

    using ProcedureList::begin;
    using ProcedureList::end;
    using ProcedureList::empty;

    const Block* entryBlock () const;
    const Block* exitBlock () const;

    void buildProcedureCFG (Procedure& proc);
    void numberInstructions ();
    void numberBlocks ();

private:

    void assignToBlocks (ICodeList& imops);
    void propagate (Procedure& proc, bool visitCalls);

};

inline std::ostream &operator<<(std::ostream& out, const Program& proc) {
    return proc.print(out);
}

} // namespace SecreC

#endif // BLOCKS_H
