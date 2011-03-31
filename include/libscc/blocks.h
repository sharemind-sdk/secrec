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
 * \todo Need something better for CFG construction and
 * manipulation (which in essense \a Blocks class is).
 */
class Blocks: public std::vector<Block*> {
    private:
        Blocks (const Blocks&); // do not implement
        void operator = (const Blocks&); // do not implement
        typedef ICodeList::const_iterator CCI;
    public: /* Types: */
        typedef std::map<const Imop*, Block*> IAB; // Imop assignment block

    public: /* Methods: */
        Blocks ()
            : m_entryBlock (0), m_exitBlock (0)
        { }
        ~Blocks();

        void init(const ICodeList &code);

        inline Block& entryBlock () const { return *m_entryBlock; }
        inline Block& exitBlock () const { return *m_exitBlock; }

        std::string toString() const;

    protected: /* Methods: */
        CCI endBlock(Block &b,
                     CCI start, CCI end,
                     IAB &from, IAB &to,
                     IAB &callFrom, IAB &callTo,
                     IAB &returnFrom, IAB &returnTo);

    private: /* Fields: */
        Block *m_entryBlock;
        Block *m_exitBlock;
};

/*******************************************************************************
  Block
*******************************************************************************/

/**
 * \brief Representation of intermediate code basic block.
 * \a Block doesn't own \a Imop and does not destroy them. \a ICodeList
 * is responsible of destroying all instructions.
 */
class Block : public std::list<Imop* > {
    Block (const Block&); // do not implement
    void operator = (const Block&); // do not implement
public:

    Block (unsigned long i)
        : callPassTo (0),
          callPassFrom (0),
          index (i),
          reachable (false)
    { }

    ~Block ();

    /// \brief unlink block from CFG
    void unlink ();

    inline Imop* firstImop() const { return front (); }
    inline Imop* lastImop() const { return back (); }

    std::set<Block*> users;
    std::set<Block*> predecessors;
    std::set<Block*> predecessorsCondFalse;
    std::set<Block*> predecessorsCondTrue;
    std::set<Block*> predecessorsCall;
    std::set<Block*> predecessorsRet;
    std::set<Block*> successors;
    std::set<Block*> successorsCondFalse;
    std::set<Block*> successorsCondTrue;
    std::set<Block*> successorsCall;
    std::set<Block*> successorsRet;
    Block *callPassTo;
    Block *callPassFrom;
    unsigned long index;
    bool reachable;
};

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Blocks &bs);

#endif // BLOCKS_H
