/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECRECC_COPYELIMINATION_H
#define SECRECC_COPYELIMINATION_H

namespace SecreC {
    class ICode;
} /* namespace SecreC { */

namespace SecreCC {

void eliminateRedundantCopies (SecreC::ICode& code);

} /* namespace SecreCC { */

#endif /* SECRECC_COPYELIMINATION_H */
