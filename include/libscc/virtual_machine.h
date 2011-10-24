#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <string>

namespace SecreC {

class Program;

class VirtualMachine {
public:
    inline VirtualMachine() { }
    void run (const Program&);
    std::string toString(void);
};

}

#endif // VIRTUAL_MACHINE_H
