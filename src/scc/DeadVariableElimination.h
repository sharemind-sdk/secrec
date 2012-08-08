/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECRECC_DEADVARIABLEELIMINATION_H
#define SECRECC_DEADVARIABLEELIMINATION_H

namespace SecreC {
    class ICode;
} /* namespace SecreC { */

namespace SecreCC {

void eliminateDeadVariables (SecreC::ICode& code);

} /* namespace SecreCC { */

#endif /* SECRECC_DEADVARIABLEELIMINATION_H */
