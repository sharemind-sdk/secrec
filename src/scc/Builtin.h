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

#include <map>

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
    virtual BuiltinFunction* clone () const = 0;
};

/*******************************************************************************
  BuiltinFunctions
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinFunctions {
public: /* Types: */

    typedef std::map<VMLabel*, BuiltinFunction* > Map;

public: /* Methods: */

    BuiltinFunctions ();
    ~BuiltinFunctions ();

    /// Add function into the pool
    void insert (VMLabel* label, const BuiltinFunction& func);

    /// Generate bodies of inserted functions
    void generateAll (VMCodeSection& code, VMSymbolTable& st);

protected:

    void eraseAll ();

private: /* Fields: */

    Map m_funtions;
};

/*******************************************************************************
  BuiltinAlloc
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinAlloc
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinAlloc (unsigned size) : m_size (size) { }
    
    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinAlloc (m_size);
    }

private: /* Fields: */
    const unsigned m_size;
};

/*******************************************************************************
  BuiltinVArith
*******************************************************************************/

/// Builtin vectorised arithmetic operations
class __attribute__ ((visibility("internal"))) BuiltinVArith
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinVArith (const SecreC::Imop* imop) : m_imop (imop) { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinVArith (m_imop);
    }

private: /* Fields: */
    const SecreC::Imop* const m_imop;
};

/*******************************************************************************
  BuiltinFloatToInt
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinFloatToInt
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinFloatToInt(VMDataType dest, VMDataType src)
        : m_dest(dest)
        , m_src(src)
    { }

    void generate(VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone() const {
        return new BuiltinFloatToInt(m_dest, m_src);
    }

private: /* Fields: */
    const VMDataType m_dest;
    const VMDataType m_src;
};

/*******************************************************************************
  BuiltinVCast
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinVCast
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinVCast (VMDataType dest, VMDataType src)
        : m_dest (dest)
        , m_src (src)
    { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinVCast (m_dest, m_src);
    }

private: /* Fields: */
    const VMDataType m_dest;
    const VMDataType m_src;
};

/*******************************************************************************
  BuiltinVBoolCast
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinVBoolCast
    : public BuiltinFunction
{
public: /* Methods: */
    BuiltinVBoolCast (VMDataType src)
        : m_src (src)
    { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinVBoolCast (m_src);
    }

private: /* Fields: */
    const VMDataType m_src;
};

/*******************************************************************************
  BuiltinStrAppend
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinStrAppend
    : public BuiltinFunction
{
public: /* Methods: */

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinStrAppend ();
    }
};

/*******************************************************************************
  BuiltinStrDup
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinStrDup
    : public BuiltinFunction
{
public: /* Methods: */

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinStrDup ();
    }
};

/*******************************************************************************
  BuiltinBoolToString
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinBoolToString
    : public BuiltinFunction
{
public: /* Methods: */
    explicit BuiltinBoolToString (StringLiterals* strLit)
        : m_strLit (strLit)
    { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinBoolToString (m_strLit);
    }

private: /* Fields: */
    StringLiterals*  const m_strLit;
};

/*******************************************************************************
  BuiltinStringCmp
*******************************************************************************/

class __attribute__ ((visibility("internal"))) BuiltinStringCmp
    : public BuiltinFunction
{
public: /* Methods: */

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinStringCmp ();
    }
};

} // namespace SecreCC

#endif
