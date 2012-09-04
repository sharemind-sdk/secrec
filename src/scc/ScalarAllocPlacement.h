/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECRECC_SCALARALLOCPLACEMENT_H
#define SECRECC_SCALARALLOCPLACEMENT_H

#include <map>
#include <set>

namespace SecreC {
    class ICode;
    class Imop;
    class Symbol;
} /* namespace SecreC { */

namespace SecreCC {

typedef std::map<const SecreC::Imop*, std::set<const SecreC::Symbol*> > AllocMap;

/**
 * @brief places appropriate RELEASE instructions for private scalars, and
 *        returns which locations the scalars needs to be allocated at
 * @note we can't place explicit allocation nodes for few reasons
 *       1) this would generate IR code with subsequent assignments
 *          that register allocator would promptly mangle to a mess
 *       2) we lack the proper IR instruction to only allocate a node
 *          as classification is both allocation and initialization
 * @return for every instructions the set of symbols that have to be allocated
 */
AllocMap placePrivateScalarAllocs (SecreC::ICode& code);

} /* namespace SecreCC { */

#endif /* SECRECC_SCALARALLOCPLACEMENT_H */
