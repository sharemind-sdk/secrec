#include "RegisterAllocator.h"
#include "VMCode.h"
#include "VMSymbolTable.h"

#include <libscc/symbol.h>

#include <map>
#include <set>

namespace {

using namespace SecreC;
using namespace SecreCC;


class RegisterAllocator {

private: /* Types: */

    typedef std::set<const Symbol* > SymbolSet;

public: /* Methods: */

    RegisterAllocator (VMSymbolTable& st)
        : m_st (st)
        , m_regCount (0)
        , m_isGlobal (true)
    { }

    ~RegisterAllocator () { }

    void allocCode (VMCode& code) {
        typedef VMCode::iterator Iter;
        for (Iter i = code.begin (), e = code.end (); i != e; ++ i) {
            VMFunction& func (*i);
            m_isGlobal = func.isStart ();
            m_regCount = 0;
            allocFunction (func);
            if (func.name () == 0) {
                code.setNumGlobals (m_regCount);
            }
            else {
                func.setNumLocals (m_regCount);
            }
        }
    }

    void allocFunction (VMFunction& func) {
        typedef VMFunction::iterator Iter;
        for (Iter i = func.begin (), e = func.end (); i != e; ++ i) {
            VMBlock& block (*i);
            allocBlock (block);
        }
    }

    void allocBlock (VMBlock& block) {
        typedef VMBlock::iterator Iter;
        for (Iter i = block.begin (), e = block.end (); i != e; ++ i) {
            VMInstruction& instuction (*i);
            allocInstruction (instuction);
        }
    }

    void allocInstruction (VMInstruction& instr) {
        typedef VMInstruction::RegSet::iterator Iter;
        for (Iter i = instr.beginDef (), e = instr.endDef (); i != e; ++ i) {
            VMVReg* reg (*i);
            if (reg->actualReg () != 0) continue;
            
            VMValue* val = 0;
            if (m_isGlobal)
                val = m_st.getReg (m_regCount ++);
            else
                val = m_st.getStack (m_regCount ++);
            reg->setActualReg (val);
        }
    }

private: /* Fields: */

    VMSymbolTable&  m_st;
    unsigned        m_regCount;
    bool            m_isGlobal;
};

} // anonymous namespace

namespace SecreCC {

using namespace SecreC;

void allocRegisters (VMCode& code, VMSymbolTable& st) {
    RegisterAllocator ra (st);
    ra.allocCode (code);
    return;
}

} // namespace SecreCC
