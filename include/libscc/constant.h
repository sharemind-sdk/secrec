#ifndef SECREC_CONSTANT_H
#define SECREC_CONSTANT_H

#include "symbol.h"

// for some reason cstdint fails
#include <stdint.h>

namespace SecreC {

/**
 * given class name, C++ value type, and secrec value type
 * constructs class to represent secrec constants of given type
 * using the appropriate C++ type.
 */
#define DECL_CONST(name, cValTy, secrecValTy)\
class Constant##name : public Symbol {\
    public:\
        explicit Constant##name (cValTy value)\
            : Symbol(Symbol::CONSTANT, TypeNonVoid(secrecValTy)), m_value(value) {}\
        inline cValTy const& value() const { return m_value; }\
        virtual std::string toString() const;\
    private:\
        const cValTy m_value;\
}\

DECL_CONST (String, std::string, DATATYPE_STRING);
DECL_CONST (Bool,   bool,        DATATYPE_BOOL);
DECL_CONST (Int8,   int8_t,      DATATYPE_INT8);
DECL_CONST (Int16,  int16_t,     DATATYPE_INT16);
DECL_CONST (Int32,  int32_t,     DATATYPE_INT32);
DECL_CONST (Int64,  int64_t,     DATATYPE_INT64);
DECL_CONST (Int,    int,         DATATYPE_INT);
DECL_CONST (UInt8,  uint8_t,     DATATYPE_UINT8);
DECL_CONST (UInt16, uint16_t,    DATATYPE_UINT16);
DECL_CONST (UInt32, uint32_t,    DATATYPE_UINT32);
DECL_CONST (UInt64, uint64_t,    DATATYPE_UINT64);
DECL_CONST (UInt,   unsigned,    DATATYPE_UINT);


#undef DECL_CONST

}

#endif
