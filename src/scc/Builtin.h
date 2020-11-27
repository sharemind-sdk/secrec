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

#ifndef BUILTIN_H
#define BUILTIN_H

#include <unordered_map>
#include <memory>

#include "VMValue.h"
#include "VMCode.h"
#include "VMSymbolTable.h"

namespace SecreC {
    class Imop;
} /* namespace SecreC { */

namespace SecreCC {

class StringLiterals;

/*******************************************************************************
  BuiltinFunction
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinFunction {
public: /* Methods: */
    virtual ~BuiltinFunction () { }
    virtual void generate (VMFunction& function, VMSymbolTable& st) = 0;
    virtual std::unique_ptr<BuiltinFunction> clone () const = 0;
};

/*******************************************************************************
  BuiltinFunctions
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinFunctions {

public: /* Methods: */

    BuiltinFunctions();
    ~BuiltinFunctions();

    /// Add function into the pool
    void insert(VMLabel * label, BuiltinFunction const & func);

    /// Generate bodies of inserted functions
    void generateAll(VMCodeSection & code, VMSymbolTable & st);

private: /* Fields: */

    std::unordered_map<VMLabel *, std::unique_ptr<BuiltinFunction>> m_functions;

};

/*******************************************************************************
  BuiltinAlloc
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinAlloc final
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinAlloc (unsigned size) : m_size (size) { }
    
    void generate(VMFunction & function, VMSymbolTable & st) final override;

    std::unique_ptr<BuiltinFunction> clone() const final override
    { return std::make_unique<BuiltinAlloc>(m_size); }

private: /* Fields: */
    const unsigned m_size;
};

/*******************************************************************************
  BuiltinVArith
*******************************************************************************/

/// Builtin vectorised arithmetic operations
class __attribute__ ((visibility("internal"))) BuiltinVArith final
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinVArith (const SecreC::Imop* imop) : m_imop (imop) { }

    void generate(VMFunction & function, VMSymbolTable & st) final override;

    std::unique_ptr<BuiltinFunction> clone() const final override
    { return std::make_unique<BuiltinVArith>(m_imop); }

private: /* Fields: */
    const SecreC::Imop* const m_imop;
};

/*******************************************************************************
  BuiltinFloatToInt
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinFloatToInt final
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinFloatToInt(VMDataType dest, VMDataType src)
        : m_dest(dest)
        , m_src(src)
    { }

    void generate(VMFunction & function, VMSymbolTable & st) final override;

    std::unique_ptr<BuiltinFunction> clone() const final override
    { return std::make_unique<BuiltinFloatToInt>(m_dest, m_src); }

private: /* Fields: */
    const VMDataType m_dest;
    const VMDataType m_src;
};

/*******************************************************************************
  BuiltinVCast
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinVCast final
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinVCast (VMDataType dest, VMDataType src)
        : m_dest (dest)
        , m_src (src)
    { }

    void generate(VMFunction & function, VMSymbolTable & st) final override;

    std::unique_ptr<BuiltinFunction> clone() const final override
    { return std::make_unique<BuiltinVCast>(m_dest, m_src); }

private: /* Fields: */
    const VMDataType m_dest;
    const VMDataType m_src;
};

/*******************************************************************************
  BuiltinVBoolCast
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinVBoolCast final
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinVBoolCast (VMDataType src)
        : m_src (src)
    { }

    void generate(VMFunction & function, VMSymbolTable & st) final override;

    std::unique_ptr<BuiltinFunction> clone() const final override
    { return std::make_unique<BuiltinVBoolCast>(m_src); }

private: /* Fields: */
    const VMDataType m_src;
};

/*******************************************************************************
  BuiltinStrAppend
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinStrAppend final
    : public BuiltinFunction
{
public: /* Methods: */

    void generate(VMFunction & function, VMSymbolTable & st) final override;

    std::unique_ptr<BuiltinFunction> clone() const final override
    { return std::make_unique<BuiltinStrAppend>(); }
};

/*******************************************************************************
  BuiltinStrDup
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinStrDup final
    : public BuiltinFunction
{
public: /* Methods: */

    void generate(VMFunction & function, VMSymbolTable & st) final override;

    std::unique_ptr<BuiltinFunction> clone() const final override
    { return std::make_unique<BuiltinStrDup>(); }
};

/*******************************************************************************
  BuiltinBoolToString
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinBoolToString final
    : public BuiltinFunction
{
public: /* Methods: */
    explicit BuiltinBoolToString (StringLiterals* strLit)
        : m_strLit (strLit)
    { }

    void generate(VMFunction & function, VMSymbolTable & st) final override;

    std::unique_ptr<BuiltinFunction> clone() const final override
    { return std::make_unique<BuiltinBoolToString>(m_strLit); }

private: /* Fields: */
    StringLiterals*  const m_strLit;
};

/*******************************************************************************
  BuiltinStringCmp
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinStringCmp final
    : public BuiltinFunction
{
public: /* Methods: */

    void generate(VMFunction & function, VMSymbolTable & st) final override;

    std::unique_ptr<BuiltinFunction> clone() const final override
    { return std::make_unique<BuiltinStringCmp>(); }
};

} // namespace SecreCC

#endif
