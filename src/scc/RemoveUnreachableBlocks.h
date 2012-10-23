/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECRECC_REMOVEUNREACHABLE_BLOCKS_H
#define SECRECC_REMOVEUNREACHABLE_BLOCKS_H

namespace SecreC {
    class ICode;
} /* namespace SecreC { */

namespace SecreCC {

void removeUnreachableBlocks (SecreC::ICode& code);

} /* namespace SecreCC { */

#endif /* SECRECC_REMOVEUNREACHABLE_BLOCKS_H */
