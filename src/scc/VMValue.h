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

#include <iosfwd>
#include <string>
#include <stdint.h>

namespace SecreCC {

/******************************************************************
  VMValue
******************************************************************/

/**
 * Representation of Sharemind virtual machine values.
 * Only (de)allocated by the VM symbol table class.
 */
class VMValue {
public: /* Methods: */

    virtual ~VMValue () { }
    
    friend std::ostream& operator << (std::ostream& os, const VMValue& value);

protected:

    virtual void printV (std::ostream& os) const = 0;
};
    
std::ostream& operator << (std::ostream& os, const VMValue& value);

/******************************************************************
  VMImm
******************************************************************/

class VMImm : public VMValue {
public: /* Methods: */

    explicit VMImm (uint64_t value)
        : m_value (value)
    { }

    uint64_t value () const { return m_value; }

protected:

    void printV (std::ostream& os) const;

private: /* Fields: */

    const uint64_t m_value;
};

/******************************************************************
  VMStack
******************************************************************/

class VMStack : public VMValue {
public: /* Methods: */

    explicit VMStack (unsigned num)
        : m_number (num)
    { }

    unsigned number () const { return m_number; }

protected:

    void printV (std::ostream& os) const;

private: /* Fields: */

    const unsigned m_number;
};

/******************************************************************
  VMReg
******************************************************************/

class VMReg : public VMValue {
public: /* Methods: */

    explicit VMReg (unsigned num)
        : m_number (num)
    { }

    unsigned number () const { return m_number; }

protected:

    void printV (std::ostream& os) const;

private: /* Fields: */

    const unsigned m_number;
};

/******************************************************************
  VMLabel
******************************************************************/

class VMLabel : public VMValue {
public: /* Methods: */

    explicit VMLabel (const std::string& name)
        : m_name (name)
    { }

    inline const std::string& name () const { return m_name; }

protected:

    void printV (std::ostream& os) const;

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
public: /* Methods: */

    explicit VMVReg (bool isGlobal)
        : m_actualReg (0)
        , m_isGlobal (isGlobal)
    { }

    std::string toString () const;
    bool isGlobal () const { return m_isGlobal; }
    VMValue* actualReg () const { return m_actualReg; }
    void setActualReg (VMValue* reg) {
        m_actualReg = reg;
    }

protected:

    void printV (std::ostream& os) const;

private: /* Fields: */

    VMValue*    m_actualReg;
    const bool  m_isGlobal;
};


} // namespace SecreCC

#endif
