#ifndef VM_CODE_H
#define VM_CODE_H

#include "VMInstruction.h"

#include <list>

namespace SecreC {
    class Block;
} // namespace SecreC

namespace SecreCC {

/*******************************************************************************
  VMBlock
*******************************************************************************/

class VMBlock {

public: /* Types: */

    typedef std::list<VMInstruction > InstList;
    typedef InstList::iterator iterator;
    typedef InstList::const_iterator const_iterator;

public: /* Methods: */

    VMBlock (const VMLabel* name, const SecreC::Block* block) 
        : m_name (name)
        , m_secrecBlock (block)
    { }

    ~VMBlock () { }

    const VMLabel* name () const { return m_name; }

    iterator begin () { return m_instructions.begin (); }
    iterator end () { return m_instructions.end (); }
    const_iterator begin () const { return m_instructions.begin (); }
    const_iterator end () const { return m_instructions.end (); }
    void push_back (const VMInstruction& i) {
        m_instructions.push_back (i);
    }

    friend std::ostream& operator << (std::ostream& os, const VMBlock& block);

private: /* Fields: */

    const VMLabel*        m_name;
    const SecreC::Block*  m_secrecBlock;
    InstList              m_instructions;
};

/*******************************************************************************
  VMFunction
*******************************************************************************/


class VMFunction {

public: /* Types: */

    typedef std::list<VMBlock > BlockList;
    typedef BlockList::iterator iterator;
    typedef BlockList::const_iterator const_iterator;

public: /* Methods: */

    explicit VMFunction (const VMLabel* name)
        : m_name (name)
        , m_isStart (false)
        , m_numLocals (0)

    { }

    ~VMFunction () { }

    const VMLabel* name () const { return m_name; }

    unsigned numLocals () const { return m_numLocals; }
    void setNumLocals (unsigned n) { m_numLocals = n; }
    void setIsStart () { m_isStart = true; }
    bool isStart () const { return m_isStart; }

    iterator begin () { return m_blocks.begin (); }
    iterator end () { return m_blocks.end (); }
    const_iterator begin () const { return m_blocks.begin (); }
    const_iterator end () const { return m_blocks.end (); }
    void push_back (const VMBlock& b) {
        m_blocks.push_back (b);
    }

    friend std::ostream& operator << (std::ostream& os, const VMFunction& function);

private: /* Fields: */

   BlockList       m_blocks;
   const VMLabel*  m_name;
   bool            m_isStart;
   unsigned        m_numLocals;
};

/*******************************************************************************
  VMCode
*******************************************************************************/

class VMCode {

public: /* Types: */

    typedef std::list<VMFunction > FunctionList;
    typedef FunctionList::iterator iterator;
    typedef FunctionList::const_iterator const_iterator;

public: /* Methods: */

    VMCode () : m_numGlobals (0) { }

    ~VMCode () { }

    unsigned numGlobals () const { return m_numGlobals; }
    void setNumGlobals (unsigned n) { m_numGlobals = n; }

    iterator begin () { return m_functions.begin (); }
    iterator end () { return m_functions.end (); }
    const_iterator begin () const { return m_functions.begin (); }
    const_iterator end () const { return m_functions.end (); }
    void push_back (const VMFunction& f) {
        m_functions.push_back (f);
    }

    friend std::ostream& operator << (std::ostream& os, const VMCode& code);

private: /* Fields: */

   FunctionList    m_functions;
   unsigned        m_numGlobals;
};

std::ostream& operator << (std::ostream& os, const VMBlock& block);
std::ostream& operator << (std::ostream& os, const VMFunction& function);
std::ostream& operator << (std::ostream& os, const VMCode& code);


} // namespace SecreCC

#endif
