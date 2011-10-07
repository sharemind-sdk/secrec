#ifndef VMINSTRUCTION_H
#define VMINSTRUCTION_H

#include "VMValue.h"

#include <vector>
#include <set>

/**
 * \todo we might need to track a use/def chains
 */

namespace SecreCC {

/******************************************************************
  VMInstruction
******************************************************************/

class VMInstruction {

public: /* Types: */

    typedef std::set<VMVReg*> RegSet;

public: /* Methods: */

    VMInstruction () { }
    ~VMInstruction () { }

    VMInstruction& arg (const char* str);
    VMInstruction& arg (VMImm* imm);
    VMInstruction& arg (VMLabel* label);
    VMInstruction& use (VMVReg* reg);
    VMInstruction& def (VMVReg* reg);

    RegSet::iterator       beginUse ()       { return m_use.begin (); }
    RegSet::const_iterator beginUse () const { return m_use.begin (); }
    RegSet::iterator       beginDef ()       { return m_def.begin (); }
    RegSet::const_iterator beginDef () const { return m_def.begin (); }
    RegSet::iterator       endUse ()         { return m_use.end();    }
    RegSet::const_iterator endUse ()   const { return m_use.end();    }
    RegSet::iterator       endDef ()         { return m_def.end();    }
    RegSet::const_iterator endDef ()   const { return m_def.end();    }

    friend std::ostream& operator << (std::ostream& o, const VMInstruction& instr);

private: /* Fields: */

    std::vector<VMValue* >     m_operands;
    std::vector<const char* >  m_strings;
    std::set<VMVReg* >         m_use, m_def;
};


std::ostream& operator << (std::ostream& o, const VMInstruction& instr);

}

#endif
