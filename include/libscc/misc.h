#ifndef SECREC_MISC_H
#define SECREC_MISC_H

#include "ParserEnums.h"

#include <cassert>
#include <string>

std::string strEscape (const std::string& input);

std::string xmlEncode(const std::string &input);

const char * SecrecFundDataTypeToString(SecrecDataType dataType);

#endif // MISC_H
