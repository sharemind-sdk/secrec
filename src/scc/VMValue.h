/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef VM_VALUE_H
#define VM_VALUE_H

#include <stdint.h>
#include <string>

namespace SecreCC {

class VMSymbolTable;

/******************************************************************
  VMValue
******************************************************************/

/**
 * Representation of Sharemind virtual machine values.
 * Only (de)allocated by the VM symbol table class.
 */
class VMValue {
    friend class VMSymbolTable;

public: /* Types: */

    enum Type {
        Imm = 0, ///< Immediate
        Stack,   ///< Stack register
        Reg,     ///< Global register
        Label,   ///< Label
        VReg     ///< Virtual Register
    };

protected: /* Methods: */

    explicit VMValue (Type type)
        : m_type (type)
    { }

public:

    virtual ~VMValue () { }
    
    virtual std::string toString () const = 0;
    friend std::ostream& operator << (std::ostream& os, const VMValue& value);

private: /* Fields: */

    const Type m_type;
};
    
std::ostream& operator << (std::ostream& os, const VMValue& value);

/******************************************************************
  VMImm
******************************************************************/

class VMImm : public VMValue {
    friend class VMSymbolTable;

protected: /* Methods: */

    VMImm (uint64_t value)
        : VMValue (VMValue::Imm)
        , m_value (value)
    { }

public:

    ~VMImm () { }

    uint64_t value () const { return m_value; }
    std::string toString () const;

private: /* Fields: */

    const uint64_t m_value;
};

/******************************************************************
  VMStack
******************************************************************/

class VMStack : public VMValue {
    friend class VMSymbolTable;

protected: /* Methods: */

    VMStack (unsigned num)
        : VMValue (VMValue::Imm)
        , m_number (num)
    { }

public:

    ~VMStack () { }

    unsigned number () const { return m_number; }
    std::string toString () const;

private: /* Fields: */

    const unsigned m_number;
};

/******************************************************************
  VMReg
******************************************************************/

class VMReg : public VMValue {
    friend class VMSymbolTable;

protected: /* Methods: */

    VMReg (unsigned num)
        : VMValue (VMValue::Imm)
        , m_number (num)
    { }

public:

    ~VMReg () { }

    unsigned number () const { return m_number; }
    std::string toString () const;

private: /* Fields: */

    const unsigned m_number;
};

/******************************************************************
  VMLabel
******************************************************************/

class VMLabel : public VMValue {
    friend class VMSymbolTable;

protected: /* Methods: */

    explicit VMLabel (const std::string& name)
        : VMValue (VMValue::Label)
        , m_name (name)
    { }

public:

    ~VMLabel () { }

    inline const std::string& name () const { return m_name; }
    std::string toString () const;

private: /* Fields: */

    const std::string m_name;
};

/******************************************************************
 VMVReg
******************************************************************/

/**
 * Virtual register.
 * Representation of unallocated registers. Will be set to concrete registers
 * at the register allocation pass.
 */
class VMVReg: public VMValue {
    friend class VMSymbolTable;

protected: /* Methods: */

    explicit VMVReg (bool isGlobal)
        : VMValue (VMValue::VReg)
        , m_actualReg (0)
        , m_isGlobal (isGlobal)
    { }

public:

    ~VMVReg () { }

    std::string toString () const;
    bool isGlobal () const { return m_isGlobal; }
    VMValue* actualReg () const { return m_actualReg; }
    void setActualReg (VMValue* reg) {
        m_actualReg = reg;
    }

private: /* Fields: */

    VMValue*    m_actualReg;
    const bool  m_isGlobal;
};


} // namespace SecreCC

#endif
