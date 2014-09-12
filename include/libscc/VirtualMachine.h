#ifndef SECREC_VIRTUAL_MACHINE_H
#define SECREC_VIRTUAL_MACHINE_H

namespace SecreC {

class Program;

class VirtualMachine {
public:
    inline VirtualMachine () { }
    int run (const Program&);
};

}

#endif // VIRTUAL_MACHINE_H