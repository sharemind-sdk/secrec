#ifndef VM_VALUE_H
#define VM_VALUE_H

#include <stdint.h>
#include <string>

namespace SecreCC {

class VMSymbolTable;

/******************************************************************
  VMValue
******************************************************************/

/// VMValues must only be constructed by VMSymbolTable
class VMValue {
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
    friend class VMSymbolTable;

private: /* Fields: */

    const Type m_type;
};
    
std::ostream& operator << (std::ostream& os, const VMValue& value);

/******************************************************************
  VMImm
******************************************************************/

class VMImm : public VMValue {

protected: /* Methods: */

    VMImm (uint64_t value)
        : VMValue (VMValue::Imm)
        , m_value (value)
    { }

public:

    ~VMImm () { }

    uint64_t value () const { return m_value; }
    std::string toString () const;
    friend class VMSymbolTable;

private: /* Fields: */

    const uint64_t m_value;
};

/******************************************************************
  VMStack
******************************************************************/

class VMStack : public VMValue {

protected: /* Methods: */

    VMStack (unsigned num)
        : VMValue (VMValue::Imm)
        , m_number (num)
    { }

public:

    ~VMStack () { }

    unsigned number () const { return m_number; }
    std::string toString () const;
    friend class VMSymbolTable;

private: /* Fields: */

    const unsigned m_number;
};

/******************************************************************
  VMReg
******************************************************************/

class VMReg : public VMValue {

protected: /* Methods: */

    VMReg (unsigned num)
        : VMValue (VMValue::Imm)
        , m_number (num)
    { }

public:

    ~VMReg () { }

    unsigned number () const { return m_number; }
    std::string toString () const;
    friend class VMSymbolTable;

private: /* Fields: */

    const unsigned m_number;
};

/******************************************************************
  VMLabel
******************************************************************/

class VMLabel : public VMValue {

protected: /* Methods: */

    explicit VMLabel (const std::string& name)
        : VMValue (VMValue::Label)
        , m_name (name)
    { }

public:

    ~VMLabel () { }

    inline const std::string& name () const { return m_name; }
    std::string toString () const;
    friend class VMSymbolTable;

private: /* Fields: */

    const std::string m_name;
};

/******************************************************************
  VMVReg
******************************************************************/


/**
 * Virtual Register.
 * Representation of unallocated registers.
 */
class VMVReg: public VMValue {

protected: /* Methods: */

    explicit VMVReg ()
        : VMValue (VMValue::VReg)
        , m_actualReg (0)
    { }

public:

    ~VMVReg () { }

    std::string toString () const;
    VMValue* actualReg () const { return m_actualReg; }
    void setActualReg (VMValue* reg) {
        m_actualReg = reg;
    }

    friend class VMSymbolTable;

private:

    VMValue* m_actualReg;
};

} // namespace SecreCC

#endif
