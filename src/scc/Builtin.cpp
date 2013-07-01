/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Builtin.h"

#include <sstream>
#include <boost/foreach.hpp>

#include <libscc/imop.h>
#include <libscc/symbol.h>

#include "VMDataType.h"
#include "StringLiterals.h"


namespace SecreCC {

/*******************************************************************************
  BuiltinFunctions
*******************************************************************************/

BuiltinFunctions::BuiltinFunctions () { }

BuiltinFunctions::~BuiltinFunctions () {
    eraseAll ();
}

void BuiltinFunctions::insert (VMLabel* label, const BuiltinFunction& func) {
    Map::iterator i = m_funtions.find (label);
    if (i == m_funtions.end ()) {
        m_funtions.insert (i, std::make_pair (label, func.clone ()));
    }
}

void BuiltinFunctions::generateAll (VMCodeSection& code, VMSymbolTable& st) {
    BOOST_FOREACH (Map::value_type& v, m_funtions) {
        VMFunction function (v.first);
        v.second->generate (function, st);
        code.push_back (function);
    }

    eraseAll ();
}

void BuiltinFunctions::eraseAll () {
    BOOST_FOREACH (Map::value_type& v, m_funtions) {
        delete v.second;
    }

    m_funtions.clear ();
}

/*******************************************************************************
  BuiltinAlloc
*******************************************************************************/

void BuiltinAlloc::generate (VMFunction& function, VMSymbolTable& st) {
    VMStack* rDefault = st.getStack (0);
    VMStack* rSize = st.getStack (1);
    VMStack* rOut = st.getStack (2);
    VMStack* rOffset = st.getStack (3);
    VMImm*   iSize = st.getImm (m_size);

    VMLabel* lBack = 0;
    {
        std::stringstream ss;
        ss << ":back_" << st.uniq ();
        lBack = st.getLabel (ss.str ());
    }

    VMLabel* lOut = 0;
    {
        std::stringstream ss;
        ss << ":out_" << st.uniq ();
        lOut = st.getLabel (ss.str ());
    }

    VMBlock entryB (0, 0);
    entryB.push_new () << "resizestack" << 4;
    entryB.push_new () << "mov imm 0x0" << rOffset;
    entryB.push_new () << "bmul uint64" << rSize << iSize;
    // Allocate size + 1 bytes in order to avoid allocating empty array:
    entryB.push_new () << "uinc uint64" << rSize;
    entryB.push_new () << "alloc" << rOut << rSize;
    entryB.push_new () << "udec uint64" << rSize;
    function.push_back (entryB);

    VMBlock middleB (lBack, 0);
    middleB.push_new () << "jge" << lOut << "uint64" << rOffset << rSize;
    middleB.push_new () << "mov" << rDefault << "mem" << rOut << rOffset << iSize;
    middleB.push_new () << "badd uint64" << rOffset << iSize;
    middleB.push_new () << "jmp" << lBack;
    function.push_back (middleB);

    VMBlock exitB (lOut, 0);
    exitB.push_new () << "return" << rOut;
    function.push_back (exitB);
}

/*******************************************************************************
  BuiltinVArith
*******************************************************************************/


void BuiltinVArith::generate (VMFunction& function, VMSymbolTable& st) {
    using namespace SecreC;
    const Imop& imop = *m_imop;
    const unsigned n = imop.nArgs ();
    assert (n > 0);
    assert (imop.isVectorized ());
    VMDataType argTy = secrecDTypeToVMDType (imop.arg1 ()->secrecType ()->secrecDataType ());
    VMDataType destTy = secrecDTypeToVMDType (imop.dest ()->secrecType ()->secrecDataType ());
    assert (argTy != VM_INVALID && destTy != VM_INVALID);
    VMImm* argSize = st.getImm (sizeInBytes (argTy));
    VMImm* destSize = st.getImm (sizeInBytes (destTy));

    VMBlock entryB (0, 0);

    entryB.push_new () << "resizestack" << (2*n + 3);

    for (unsigned i = 0; i < 3; ++ i) {
        entryB.push_new () << "mov imm 0x0" << st.getStack (n + i);
    }

    std::vector<VMStack*> rTmp (n - 1);
    VMStack* rSize = st.getStack (n - 1);
    VMStack* rOffArg = st.getStack (n);
    VMStack* rOffDest = st.getStack (n + 1);
    VMStack* rCount = st.getStack (n + 2);
    for (unsigned i = 0; i < n-1; ++ i)
        rTmp[i] = st.getStack (n + 3 + i);


    VMLabel* lBack = 0;
    { std::stringstream ss;
      ss << ":back_" << st.uniq ();
      lBack = st.getLabel (ss.str ());
    }

    VMLabel* lOut = 0;
    { std::stringstream ss;
      ss << ":out_" << st.uniq ();
      lOut = st.getLabel (ss.str ());
    }

    // jump out if needed
    VMBlock middleB (lBack, 0);
    middleB.push_new () << "jge" << lOut << "uint64" << rCount << rSize;

    // move arguments to temporaries
    for (unsigned i = 1; i < n - 1; ++ i) {
        middleB.push_new ()
            << "mov mem"
            << st.getStack (i)
            << rOffArg
            << rTmp[i]
            << argSize;
    }

    // perform operation on temporaries
    {
        const char* name = 0;
        switch (imop.type ()) {
        case Imop::UMINUS: name = "bneg"; break;
        case Imop::UNEG  : name = "bnot"; break;
        case Imop::UINV  : name = "binv"; break;
        case Imop::MUL   : name = "tmul"; break;
        case Imop::DIV   : name = "tdiv"; break;
        case Imop::MOD   : name = "tmod"; break;
        case Imop::ADD   : name = "tadd"; break;
        case Imop::SUB   : name = "tsub"; break;
        case Imop::LAND  : name = "ltand"; break;
        case Imop::LOR   : name = "ltor"; break;
        case Imop::EQ    : name = "teq"; break;
        case Imop::NE    : name = "tne"; break;
        case Imop::LE    : name = "tle"; break;
        case Imop::LT    : name = "tlt"; break;
        case Imop::GE    : name = "tge"; break;
        case Imop::GT    : name = "tgt"; break;
        case Imop::BOR   : name = "btor"; break;
        case Imop::XOR   : name = "btxor"; break;
        case Imop::BAND  : name = "btand"; break;
        default:
            assert (false && "Not an arithmetic instruction!");
        }

        VMInstruction instr;
        instr << name << argTy;
        for (unsigned i = 0; i < n - 1; ++ i)
            instr << rTmp [i];
        middleB.push_back (instr);
    }

    // move result to memory
    middleB.push_new ()
        << "mov"
        << rTmp[0]
        << "mem"
        << st.getStack (0)
        << rOffDest
        << destSize;

    // increment offsets and counter
    middleB.push_new () << "badd uint64" << rOffDest << destSize;
    middleB.push_new () << "badd uint64" << rOffArg << argSize;
    middleB.push_new () << "uinc uint64" << rCount;

    // jump back to conditional
    middleB.push_new () << "jmp" << lBack;

    VMBlock returnB (lOut, 0);
    returnB.push_new () << "return imm 0x0";

    function.push_back (entryB);
    function.push_back (middleB);
    function.push_back (returnB);
}

/*******************************************************************************
  BuiltinVCast
*******************************************************************************/

void BuiltinVCast::generate (VMFunction& function, VMSymbolTable& st) {
    VMImm* srcSize = st.getImm (sizeInBytes (m_src));
    VMImm* destSize = st.getImm (sizeInBytes (m_dest));
    VMStack* dest = st.getStack (0);
    VMStack* src = st.getStack (1);
    VMStack* size = st.getStack (2);
    VMStack* srcOff = st.getStack (3);
    VMStack* destOff = st.getStack (4);
    VMStack* temp = st.getStack (5);

    VMLabel* middleL = st.getUniqLabel ();
    VMLabel* exitL = st.getUniqLabel ();

    ///////////////
    // Entry block:
    VMBlock entryB (0, 0);
    entryB.push_new () << "resizestack" << 6;
    entryB.push_new () << "mov" << st.getImm (0) << srcOff;
    entryB.push_new () << "mov" << st.getImm (0) << destOff;
    entryB.push_new () << "bmul" << VM_UINT64 << size << srcSize;
    entryB.push_new () << "jge" << exitL << VM_UINT64 << srcOff << size;


    VMBlock middleB (middleL, 0);
    middleB.push_new () << "mov" << "mem" << src << srcOff << temp << srcSize;
    middleB.push_new () << "convert" << m_src << temp << m_dest << temp;
    middleB.push_new () << "mov" << temp << "mem" << dest << destOff << destSize;
    middleB.push_new () << "badd" << VM_UINT64 << srcOff << srcSize;
    middleB.push_new () << "badd" << VM_UINT64 << destOff << destSize;
    middleB.push_new () << "jlt" << middleL << VM_UINT64 << srcOff << size;

    VMBlock exitB (exitL, 0);
    exitB.push_new () << "return imm 0x0";

    function.push_back (entryB)
            .push_back (middleB)
            .push_back (exitB);
    return;
}

/*******************************************************************************
  BuiltinVBoolCast
*******************************************************************************/

void BuiltinVBoolCast::generate (VMFunction& function, VMSymbolTable& st) {
    VMImm* srcSize = st.getImm (sizeInBytes (m_src));
    VMImm* destSize = st.getImm (sizeInBytes (
        secrecDTypeToVMDType (DATATYPE_BOOL)));
    VMStack* dest = st.getStack (0);
    VMStack* src = st.getStack (1);
    VMStack* size = st.getStack (2);
    VMStack* srcOff = st.getStack (3);
    VMStack* destOff = st.getStack (4);
    VMStack* temp = st.getStack (5);

    VMLabel* middleL = st.getUniqLabel ();
    VMLabel* exitL = st.getUniqLabel ();

    ///////////////
    // Entry block:
    VMBlock entryB (0, 0);
    entryB.push_new () << "resizestack" << 6;
    entryB.push_new () << "mov" << st.getImm (0) << srcOff;
    entryB.push_new () << "mov" << st.getImm (0) << destOff;
    entryB.push_new () << "bmul uint64" << size << srcSize;
    entryB.push_new () << "jge" << exitL << "uint64" << srcOff << size;

    VMBlock middleB (middleL, 0);
    middleB.push_new () << "mov" << "mem" << src << srcOff << temp << srcSize;
    middleB.push_new () << "bgt" << m_src << temp << st.getImm (0);
    middleB.push_new () << "mov" << temp << "mem" << dest << destOff << destSize;
    middleB.push_new () << "badd uint64" << srcOff << srcSize;
    middleB.push_new () << "badd uint64" << destOff << destSize;
    middleB.push_new () << "jlt" << middleL << "uint64" << srcOff << size;

    VMBlock exitB (exitL, 0);
    exitB.push_new () << "return imm 0x0";

    function.push_back (entryB)
            .push_back (middleB)
            .push_back (exitB);
    return;
}

/*******************************************************************************
  BuiltinStrAppend
*******************************************************************************/

void BuiltinStrAppend::generate (VMFunction& function, VMSymbolTable& st) {
    VMImm* lhs = st.getImm (0);
    VMImm* rhs = st.getImm (1);
    VMStack* lhsSize = st.getStack (0);
    VMStack* rhsSize = st.getStack (1);
    VMStack* totalSize = st.getStack (2);
    VMStack* dest = st.getStack (3);

    VMBlock block (0, 0);
    block.push_new () << "resizestack 0x4";
    block.push_new () << "getcrefsize" << lhs << lhsSize;
    block.push_new () << "getcrefsize" << rhs << rhsSize;
    block.push_new () << "udec" << VM_UINT64 << lhsSize;
    block.push_new () << "tadd" << VM_UINT64 << totalSize << lhsSize << rhsSize;
    block.push_new () << "alloc" << dest << totalSize;
    block.push_new () << "mov cref 0x0" << st.getImm (0) << "mem" << dest << st.getImm (0) << lhsSize;
    block.push_new () << "mov cref 0x1" << st.getImm (0) << "mem" << dest << lhsSize << rhsSize;
    block.push_new () << "return" << dest;
    function.push_back (block);
    return;
}

/*******************************************************************************
  BuiltinStrDup
*******************************************************************************/

void BuiltinStrDup::generate (VMFunction& function, VMSymbolTable& st) {
    VMImm* src = st.getImm (0);
    VMStack* size = st.getStack (0);
    VMStack* dest = st.getStack (1);

    VMBlock block (0, 0);
    block.push_new () << "resizestack 0x2";
    block.push_new () << "getcrefsize" << src << size;
    block.push_new () << "alloc" << dest << size;
    block.push_new () << "mov cref 0x0" << st.getImm (0) << "mem" << dest << st.getImm (0) << size;
    block.push_new () << "return" << dest;
    function.push_back (block);
    return;
}

/*******************************************************************************
  BuiltinBoolToString
*******************************************************************************/

void BuiltinBoolToString::generate (VMFunction& function, VMSymbolTable& st) {
    VMLabel* trueLit = m_strLit->insert ("true").label;
    VMLabel* falseLit = m_strLit->insert ("false").label;
    VMStack* value = st.getStack (0);
    VMStack* label = st.getStack (1);
    VMStack* size = st.getStack (3);
    VMStack* rodata = st.getStack (5);
    VMStack* dest = st.getStack (4);
    VMLabel* trueL = st.getUniqLabel ();

    VMBlock entryB (0, 0);
    entryB.push_new () << "resizestack" << 6;
    entryB.push_new () << "mov" << trueLit << label;
    entryB.push_new () << "mov" << st.getImm (5) << size;
    entryB.push_new () << "mov imm :RODATA" << rodata;
    entryB.push_new () << "bband" << VM_UINT64 << value << st.getImm (1);
    entryB.push_new () << "jnz" << trueL << VM_UINT64 << value;
    entryB.push_new () << "mov" << falseLit << label;
    entryB.push_new () << "mov" << st.getImm (6) << size;

    VMBlock exitB (trueL, 0);
    exitB.push_new () << "alloc" << dest << size;
    exitB.push_new () << "mov" << "mem" << rodata << label << "mem" << dest << st.getImm (0) << size;
    exitB.push_new () << "return" << dest;

    function.push_back (entryB)
            .push_back (exitB);
    return;
}


/*******************************************************************************
  BuiltinStringCmp
*******************************************************************************/

void BuiltinStringCmp::generate (VMFunction& function, VMSymbolTable& st) {
    VMStack* idx = st.getStack (0);
    VMStack* chrL = st.getStack (2);
    VMStack* chrR = st.getStack (3);
    VMStack* sizeL = st.getStack (4);
    VMStack* sizeR = st.getStack (5);
    VMStack* temp = st.getStack (6);

    VMBlock entryB (0, 0);
    entryB.push_new () << "resizestack" << 7;
    entryB.push_new () << "mov" << st.getImm (0) << idx;
    // zero the bytes to use wider subtraction
    entryB.push_new () << "mov" << st.getImm (0) << chrL;
    entryB.push_new () << "mov" << st.getImm (0) << chrR;
    entryB.push_new () << "getcrefsize" << st.getImm (0) << sizeL;
    entryB.push_new () << "getcrefsize" << st.getImm (1) << sizeR;

    VMLabel* loopL = st.getUniqLabel ();
    VMLabel* falseL = st.getUniqLabel ();
    VMLabel* trueL = st.getUniqLabel ();

    VMBlock loopB (loopL, 0);
    loopB.push_new () << "mov cref 0x0" << idx << chrL << st.getImm (1);
    loopB.push_new () << "mov cref 0x1" << idx << chrR << st.getImm (1);
    loopB.push_new () << "tsub" << VM_INT64 << temp << chrL << chrR;
    loopB.push_new () << "jnz" << falseL << VM_UINT64 << temp;
    loopB.push_new () << "uinc" << VM_UINT64 << idx;
    loopB.push_new () << "jge" << trueL << VM_UINT64 << idx << sizeL;
    loopB.push_new () << "jge" << trueL << VM_UINT64 << idx << sizeR;
    loopB.push_new () << "jmp" << loopL;

    VMBlock trueB (trueL, 0);
    loopB.push_new () << "tsub" << VM_INT64 << temp << sizeL << sizeR;

    VMBlock falseB (falseL, 0);
    falseB.push_new () << "return" << temp;

    function.push_back (entryB)
            .push_back (loopB)
            .push_back (trueB)
            .push_back (falseB);
}


} // namespace SecreCC
