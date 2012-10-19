/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef VMINSTRUCTION_H
#define VMINSTRUCTION_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <ostream>
#include <string>
#include <vector>

#include "VMDataType.h"

namespace SecreCC {

class VMValue;

/******************************************************************
  VMInstruction
******************************************************************/

class VMInstruction {
private: /* Types: */

    struct Operand {
        friend class VMInstruction;
    private: /* Types: */
        enum Type {
            Value = 0,
            Number,
            String
        };

    private: /* Methods: */
        explicit Operand (const VMValue& value)
            : m_type (Value)
            , un_value (&value)
        { }

        explicit Operand (uint64_t number)
            : m_type (Number)
            , un_number (number)
        { }

        explicit Operand (const char* str)
            : m_type (String)
            , un_string (str)
        { }

    private: /* Fields: */
        Type m_type;
        union {
            const VMValue* un_value;
            uint64_t       un_number;
            const char*    un_string;
        };
    };

    /// Workaround to avoid implicit const char* to std::string conversion.
    struct StringOperand {
        friend class VMInstruction;
    private: /* Methods: */
        explicit StringOperand (const std::string& str)
            : m_str (str)
        { }
    private: /* Fields: */
        const std::string& m_str;
    };

public: /* Methods: */

    VMInstruction& operator << (const StringOperand& str) { return arg (str); }
    VMInstruction& operator << (const char* str) { return arg (str); }
    VMInstruction& operator << (VMValue* val) { return arg (val); }
    VMInstruction& operator << (uint64_t n) { return arg (n); }
    VMInstruction& operator << (VMDataType ty) { return arg (dataTypeToStr (ty)); }

    /**
     * Default we assume that instruction fragment is a string literal, use this to pass
     * a string parameter if the string might not live as long as the instruction.
     */
    inline static const StringOperand str (const std::string& str) { return StringOperand (str); }

private:

    VMInstruction& arg (const StringOperand& str);
    VMInstruction& arg (const char* str);
    VMInstruction& arg (VMValue* val);
    VMInstruction& arg (uint64_t n);

    void print (std::ostream& os) const;

    friend std::ostream& operator << (std::ostream& o, const VMInstruction& instr);

private: /* Fields: */

    std::vector<Operand > m_operands;
    static boost::ptr_vector<std::string> m_allocStrings;
};


std::ostream& operator << (std::ostream& o, const VMInstruction& instr);

}

#endif
