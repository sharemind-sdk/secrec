/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef ARGS_H
#define ARGS_H

namespace SecreCC {

// top level commands
namespace Flag {

enum Name {
    Verbose = 0,
    Help,
    Output,
    Optimize,
    Count
};

}

extern int flags [Flag::Count];
void help (void);

}

#endif
