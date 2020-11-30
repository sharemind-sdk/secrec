/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#ifndef VMINSTRUCTION_H
#define VMINSTRUCTION_H

#include <ostream>
#include <string>
#include <vector>
#include <stdint.h>

#include "VMDataType.h"

namespace SecreCC {

class VMValue;

/******************************************************************
  VMInstruction
******************************************************************/

class __attribute__ ((visibility("internal"))) VMInstruction {
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
};


std::ostream& operator << (std::ostream& o, const VMInstruction& instr)
        __attribute__((visibility("internal")));

}

#endif
