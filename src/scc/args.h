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
