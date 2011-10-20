/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "TargetInfo.h"

namespace SecreCC {

const char* secrecDTypeToVMDType (SecrecDataType dtype) {
    switch (dtype) {
        case DATATYPE_INT8:   return "int8";
        case DATATYPE_INT16:  return "int16";
        case DATATYPE_INT32:  return "int32";
        case DATATYPE_INT64:  return "int64";
        case DATATYPE_INT:    return "int64";
        case DATATYPE_UINT8:  return "uint8";
        case DATATYPE_UINT16: return "uint16";
        case DATATYPE_UINT32: return "uint32";
        case DATATYPE_UINT64: return "uint64";
        case DATATYPE_UINT:   return "uint64";
        case DATATYPE_BOOL:   return "uint64";
        default:
            assert (false);
            break;
    }

    return 0;
}

unsigned secrecDTypeSize (SecrecDataType dtype) {
    switch (dtype) {
        case DATATYPE_INT8:   return 1;
        case DATATYPE_INT16:  return 2;
        case DATATYPE_INT32:  return 4;
        case DATATYPE_INT64:  return 8;
        case DATATYPE_INT:    return 8;
        case DATATYPE_UINT8:  return 1;
        case DATATYPE_UINT16: return 2;
        case DATATYPE_UINT32: return 4;
        case DATATYPE_UINT64: return 8;
        case DATATYPE_UINT:   return 8;
        case DATATYPE_BOOL:   return 8;
        default:
            assert (false);
            break;
    }

    return 0;
}

} // namespace SecreCC
