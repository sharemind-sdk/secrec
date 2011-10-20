/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef TARGET_INFO_H
#define TARGET_INFO_H

#include <libscc/types.h>

namespace SecreCC {

unsigned secrecDTypeSize (SecrecDataType dtype);
const char* secrecDTypeToVMDType (SecrecDataType dtype);

} // namespace SecreCC

#endif
