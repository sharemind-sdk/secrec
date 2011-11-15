/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "VMCode.h"
#include "VMValue.h"

#include <cassert>
#include <ostream>
#include <iterator>

namespace SecreCC {

/*******************************************************************************
  VMBlock
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMBlock& block) {
    if (block.name () != 0) {
        os << block.name ()->name () << '\n';
    }

    std::copy (block.begin (), block.end (),
               std::ostream_iterator<VMInstruction>(os, "\n"));
    return os;
}

/*******************************************************************************
  VMFunction
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMFunction& function) {
    assert (function.name () != 0);
    os << function.name ()->name () << '\n';
    if (function.numLocals () != 0) {
        assert (! function.isStart () && "Must not have local registers in global scope");
        os << "resizestack 0x" << std::hex  << function.numLocals () << '\n';
    }

    std::copy (function.begin (), function.end (),
               std::ostream_iterator<VMBlock>(os, "\n"));
    return  os;
}

/*******************************************************************************
  VMBinding
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMBinding& binding) {
    os << binding.m_label->name () << " .bind_syscall \"" << binding.m_syscall << "\"";
    return  os;
}

/*******************************************************************************
  VMCode
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMCode& code) {
    if (! code.m_bindings.empty ()) {
        os << ".section BIND\n";
        std::copy (code.m_bindings.begin (), code.m_bindings.end (),
                   std::ostream_iterator<VMBinding>(os, "\n"));
    }

    os << ".section TEXT\n";
    if (code.numGlobals () > 0) {
        os << "resizestack 0x" << std::hex  << code.numGlobals () << '\n';
    }

    std::copy (code.begin (), code.end (),
               std::ostream_iterator<VMFunction>(os, "\n"));

    return os;
}

} // namespace SecreCC
