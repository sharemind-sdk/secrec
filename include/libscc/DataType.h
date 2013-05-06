/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_DATATYPE_H
#define SECREC_DATATYPE_H

#include "parser.h"

namespace SecreC {

SecrecDataType upperDataType (SecrecDataType a, SecrecDataType b);
SecrecDimType upperDimType (SecrecDimType n, SecrecDimType m);

bool latticeDataTypeLEQ (SecrecDataType a, SecrecDataType b);
bool latticeDimTypeLEQ (SecrecDimType n, SecrecDimType m);
bool latticeExplicitLEQ (SecrecDataType a, SecrecDataType b);

bool isFloatingDataType (SecrecDataType dType);
bool isNumericDataType (SecrecDataType dType);
bool isXorDataType (SecrecDataType dType);
bool isSignedNumericDataType (SecrecDataType dType);
bool isUnsignedNumericDataType (SecrecDataType dType);

SecrecDataType dtypeDeclassify (SecrecDataType dtype);

} // namespace SecreC

#endif // SECREC_DATATYPE_H
