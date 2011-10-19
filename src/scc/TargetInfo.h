#ifndef TARGET_INFO_H
#define TARGET_INFO_H

#include <libscc/types.h>

namespace SecreCC {

unsigned secrecDTypeSize (SecrecDataType dtype);
const char* secrecDTypeToVMDType (SecrecDataType dtype);

} // namespace SecreCC

#endif
