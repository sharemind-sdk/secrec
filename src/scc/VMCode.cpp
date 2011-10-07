#include "VMCode.h"

#include <cassert>
#include <ostream>
#include <iterator>

namespace SecreCC {

/*******************************************************************************
  VMBlock
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMBlock& block) {
    assert (block.name () != 0);
    os << block.name ()->toString () << '\n';
    std::copy (block.begin (), block.end (),
               std::ostream_iterator<VMInstruction>(os, "\n"));
    return os;
}

/*******************************************************************************
  VMFunction
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMFunction& function) {
    assert (function.name () != 0);
    os << function.name ()->toString () << '\n';
    if (function.numLocals () != 0) // kind of hack, maybe add some postprocessing after RA pass?
        os << "resizestack 0x" << std::hex << function.numLocals () << '\n';
    std::copy (function.begin (), function.end (),
               std::ostream_iterator<VMBlock>(os, "\n"));
    return  os;
}

/*******************************************************************************
  VMCode
*******************************************************************************/

std::ostream& operator << (std::ostream& os, const VMCode& code) {
    os << ".section TEXT\n";
    std::copy (code.begin (), code.end (),
               std::ostream_iterator<VMFunction>(os, "\n"));

    return os;
}

} // namespace SecreCC
