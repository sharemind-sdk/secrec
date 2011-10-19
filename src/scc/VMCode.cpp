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
  VMCode
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMCode& code) {
    os << ".section TEXT\n";
    if (code.numGlobals () > 0) {
        os << "resizestack 0x" << std::hex  << code.numGlobals () << '\n';
    }

    std::copy (code.begin (), code.end (),
               std::ostream_iterator<VMFunction>(os, "\n"));

    return os;
}

} // namespace SecreCC
