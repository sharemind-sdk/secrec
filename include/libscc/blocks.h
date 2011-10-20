#ifndef BLOCKS_H
#define BLOCKS_H

#include <list>
#include <map>
#include <set>
#include <vector>

#include "icodelist.h"


namespace SecreC {

/*******************************************************************************
  Blocks
*******************************************************************************/

class Block;
class Imop;
class ReachingDefinitions;
class ReachingJumps;


/**
 * \brief In essense this is the control flow graph.
 * It deallocates all blocks on exit.
 */
class Blocks : public std::vector<Block*> {
    private:
        Blocks (const Blocks&); // do not implement
        void operator = (const Blocks&); // do not implement

    public: /* Types: */

        typedef std::map<const SymbolProcedure*, std::list<Block* > > ProcMap;

    public: /* Methods: */

        Blocks ()
            : m_entryBlock (0), m_exitBlock (0)
        { }

        ~Blocks();

        void init(ICodeList &code);

        inline Block& entryBlock () const { return *m_entryBlock; }
        inline Block& exitBlock () const { return *m_exitBlock; }

        void addBlockToProc (const SymbolProcedure* proc, Block* b);

        ProcMap::iterator beginProc ();
        ProcMap::const_iterator beginProc () const;

        ProcMap::iterator endProc ();
        ProcMap::const_iterator endProc () const;


        std::string toString() const;

    protected: /* Methods: */

        /// \brief Assign each instruction to basic block, and constructs the basic blocks.
        /// \todo get rid of nextBlock argument
        void assignToBlocks (ICodeList& imops, std::map<Block*, Block*>& nextBlock);

        /// \brief Traverse the blocks and propagate successor/predecessor information
        /// to visited blocks. \a nextBlock maps each block to it's successor if it happens
        /// to fall through.
        void propagate (const std::map<Block*, Block*>& nextBlock);

    private: /* Fields: */
        ProcMap  m_procBlocks;
        Block*   m_entryBlock;
        Block*   m_exitBlock;
};

/*******************************************************************************
  Block
*******************************************************************************/

/**
 * \brief Representation of intermediate code basic block.
 * \a Block doesn't own \a Imop and does not destroy them. \a ICodeList
 * is responsible of destroying all instructions.
 * \todo This class requires major refactoring.
 */
class Block : public ImopList {

    Block (const Block&); // do not implement
    void operator = (const Block&); // do not implement

public: /* Types: */
    
    typedef std::set<Block*> Set;

public: /* Methods: */

    explicit Block (unsigned long i)
        : m_callPassTo (0)
        , m_callPassFrom (0)
        , m_parent (0)
        , m_index (i)
        , m_reachable (false)
    { }

    ~Block ();

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
    const std::set<Block*>& succ () const { return m_successors; }
    const std::set<Block*>& succCondFalse () const { return m_successorsCondFalse; }
    const std::set<Block*>& succCondTrue () const { return m_successorsCondTrue; }
    const std::set<Block*>& succCall () const { return m_successorsCall; }
    const std::set<Block*>& succRet () const { return m_successorsRet; }
    Block* callPassTo () const { return m_callPassTo; }

    // Successors:
    void addSucc (Block* block) { m_successors.insert (block); }
    void addSuccCondFalse (Block* block) { m_successorsCondFalse.insert (block); }
    void addSuccCondTrue (Block* block) { m_successorsCondTrue.insert (block); }
    void addSuccCall (Block* block) { m_successorsCall.insert (block); }
    void addSuccRet (Block* block) { m_successorsRet.insert (block); }
    void setCallPassFrom (Block* block) { m_callPassFrom = block; }
    const std::set<Block*>& pred () const { return m_predecessors; }
    const std::set<Block*>& predCondFalse () const { return m_predecessorsCondFalse; }
    const std::set<Block*>& predCondTrue () const { return m_predecessorsCondTrue; }
    const std::set<Block*>& predCall () const { return m_predecessorsCall; }
    const std::set<Block*>& predRet () const { return m_predecessorsRet; }
    Block* callPassFrom () const { return m_callPassFrom; }

    void setReachable () { m_reachable = true; }
    bool reachable () const { return m_reachable; }
    unsigned index () const { return m_index; }
    void setParent (const SymbolProcedure* parent) { m_parent = parent; }
    const SymbolProcedure* parent () const { return m_parent; }
    
    void getIncoming (std::set<Block*>& list) const;
    void getOutgoing (std::set<Block*>& list) const;

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
    const SymbolProcedure*  m_parent;
    const unsigned long     m_index;
    bool                    m_reachable;
};

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Blocks &bs);

#endif // BLOCKS_H
