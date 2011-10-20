/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "args.h"

#include <iostream>

namespace SecreCC {

void help (void) {
  std::cout <<
  "Usage: scc [options] [FILE]\n"
  "Options:\n"
  "  -h, --help           display this help\n"
  "  -v, --verbose        print extra information during compilation\n"
  "  -o, --output=[FILE]  specify output file\n"
  "  -O                   perform some optimization\n"
  << std::endl;
}

int flags[Flag::Count];


} // namespace SecreCC
