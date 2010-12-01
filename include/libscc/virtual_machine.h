#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <string>

#include "symboltable.h"
#include "imop.h"
#include "types.h"
#include "log.h"

namespace SecreC {

class ICodeList;

class VirtualMachine {
public:
    inline VirtualMachine() { }
    void run (ICodeList const&);
    std::string toString(void);
};

}

#endif // VIRTUAL_MACHINE_H
