#ifndef BLOCKS_H
#define BLOCKS_H

#include <map>
#include <set>
#include <vector>
#include "icodelist.h"


namespace SecreC {

/*******************************************************************************
  Blocks
*******************************************************************************/

class Block;
class ReachingDefinitions;
class ReachingJumps;

class Blocks: public std::vector<Block*> {
    public: /* Types: */
        typedef ICodeList::const_iterator CCI;
        typedef std::map<const SecreC::Imop*, SecreC::Block*> IAB; // Imop assignment block

    public: /* Methods: */
        ~Blocks();

        void init(const ICodeList &code);

        inline const Block &entryBlock() const { return *m_entryBlock; }
        inline CCI imopsBegin() const;
        inline CCI imopsEnd() const;

        std::string toString() const;

    protected: /* Methods: */
         CCI endBlock(SecreC::Block &b, CCI end,
                      IAB &from, IAB &to,
                      IAB &callFrom, IAB &callTo,
                      IAB &returnFrom, IAB &returnTo);

    private: /* Fields: */
        Block *m_entryBlock;

};

/*******************************************************************************
  Block
*******************************************************************************/

struct Block {
    inline Block(ICodeList::const_iterator codestart, unsigned long i)
        : start(codestart), end(codestart), callPassTo(0), callPassFrom(0),
          index(i), reachable(false) {}

    inline Imop *operator[](size_t n) const { return *(start + n); }
    inline size_t size() const { return end - start; }
    inline Imop *firstImop() const { return *start; }
    inline Imop *lastImop() const { return *(end - 1); }

    Blocks::CCI start;
    Blocks::CCI end;
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

inline Blocks::CCI Blocks::imopsBegin() const { return front()->start; }
inline Blocks::CCI Blocks::imopsEnd() const { return back()->end; }

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Blocks &bs);

#endif // BLOCKS_H
