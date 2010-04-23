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

class Blocks {
    public: /* Types: */
        enum Status { NOT_READY, ERROR, OK };
        typedef ICodeList::const_iterator CCI;
        typedef std::map<const SecreC::Imop*, SecreC::Block*> IAB; // Imop assignment block


    public: /* Methods: */
        inline Blocks()
            : m_status(NOT_READY) {};
        ~Blocks();

        Status init(const ICodeList &code);

        const std::vector<Block*> &blocks() const { return m_blocks; }
        const Block &entryBlock() const { return *m_entryBlock; }

        std::string toString() const;
        inline Status status() const { return m_status; }

    protected: /* Methods: */
         CCI endBlock(SecreC::Block &b, CCI end,
                      IAB &from, IAB &to,
                      IAB &callFrom, IAB &callTo,
                      IAB &returnFrom, IAB &returnTo);

    private: /* Fields: */
        std::vector<Block*> m_blocks;
        Block *m_entryBlock;
        Status m_status;

};

/*******************************************************************************
  Block
*******************************************************************************/

struct Block {
    inline Block(ICodeList::const_iterator codestart, unsigned long i)
        : start(codestart), end(codestart), index(i), reachable(false) {}

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
    unsigned long index;
    bool reachable;
};

} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::Blocks &bs);

#endif // BLOCKS_H
