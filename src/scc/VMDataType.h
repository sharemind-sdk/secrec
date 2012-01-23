/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef VM_DATA_TYPE_H
#define VM_DATA_TYPE_H

#include <libscc/types.h>

namespace SecreCC {

enum VMDataType {
    VM_INVALID  = 0x00,
    VM_INT8     = 0x01,
    VM_INT16    = 0x02,
    VM_INT32    = 0x03,
    VM_INT64    = 0x04,
    VM_UINT8    = 0x11,
    VM_UINT16   = 0x12,
    VM_UINT32   = 0x13,
    VM_UINT64   = 0x14,
    VM_FLOAT32  = 0x23
};

inline unsigned sizeInBytes (VMDataType dty) { return 1 << ((dty & 0x7) - 1); }
inline bool isSigned (VMDataType dty)   { return (dty & 0x78) == 0x00; }
inline bool isUnsigned (VMDataType dty) { return (dty & 0x78) == 0x10; }
inline bool isFloating (VMDataType dty) { return (dty & 0x78) == 0x20; }

inline VMDataType secrecDTypeToVMDType (SecrecDataType dtype) {
    switch (dtype) {
    case DATATYPE_INT8:   return VM_INT8;
    case DATATYPE_INT16:  return VM_INT16;
    case DATATYPE_INT32:  return VM_INT32;
    case DATATYPE_INT64:  return VM_INT64;
    case DATATYPE_INT:    return VM_INT64;
    case DATATYPE_UINT8:  return VM_UINT8;
    case DATATYPE_UINT16: return VM_UINT16;
    case DATATYPE_UINT32: return VM_UINT32;
    case DATATYPE_UINT64: return VM_UINT64;
    case DATATYPE_UINT:   return VM_UINT64;
    case DATATYPE_BOOL:   return VM_UINT64;
    default:              return VM_INVALID;
    }
}

inline const char* dataTypeToStr (VMDataType dty) {
    switch (dty) {
    case VM_INVALID:  return "invalid";
    case VM_INT8:     return "int8";
    case VM_INT16:    return "int16";
    case VM_INT32:    return "int32";
    case VM_INT64:    return "int64";
    case VM_UINT8:    return "uint8";
    case VM_UINT16:   return "uint16";
    case VM_UINT32:   return "uint32";
    case VM_UINT64:   return "uint64";
    case VM_FLOAT32:  return "float32";
    }
}

} // namespace SecreCC

#endif