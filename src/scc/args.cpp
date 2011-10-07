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
  << std::endl;
}

int flags[Flag::Count];


} // namespace SecreCC
