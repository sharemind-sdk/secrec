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

#ifndef CONTEXT_IMPL_H
#define CONTEXT_IMPL_H

#include "ParserEnums.h"
#include "Types.h"
#include "Constant.h"
#include "StringTable.h"
#include "TypeArgument.h"
#include "SecurityType.h"

#include <map>
#include <tuple>

namespace SecreC {

class DataType;
class DataTypePrimitive;
class DataTypeStruct;
class PrivateSecType;
class PublicSecType;

class ContextImpl {
private:

    ContextImpl (const ContextImpl&) = delete;
    ContextImpl& operator = (const ContextImpl&) = delete;

public: /* Types: */

    using ConstantStringMap = std::map<StringRef, ConstantString*>;
    using NumericConstantMap = std::map<APInt, ConstantInt*>;
    using FloatConstantMap = std::map<APFloat, ConstantFloat*, APFloat::BitwiseCmp>;

public: /* Methods: */

    ContextImpl () { }

    ~ContextImpl ();

    StringTable& stringTable () { return m_stringTable; }

public: /* Fields: */

    /* Strings: */
    StringTable           m_stringTable;

    /* All constants: */
    ConstantStringMap     m_stringLiterals;
    NumericConstantMap    m_numericConstants[2]; ///< 0 - unsigned, 1 - signed
    FloatConstantMap      m_floatConstants;
};

} // namespace SecreC

#endif // CONTEXT_IMPL_H
