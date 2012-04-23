/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef BUILTIN_H
#define BUILTIN_H

#include <map>

#include "VMValue.h"
#include "VMCode.h"
#include "VMSymbolTable.h"

namespace SecreC {
    class Imop;
}

namespace SecreCC {

class StringLiterals;

/*******************************************************************************
  BuiltinFunction
*******************************************************************************/

class BuiltinFunction {
public: /* Methods: */
    BuiltinFunction () { }
    virtual ~BuiltinFunction () { }
    virtual void generate (VMFunction& function, VMSymbolTable& st) = 0;
    virtual BuiltinFunction* clone () const = 0;
};

/*******************************************************************************
  BuiltinFunctions
*******************************************************************************/

class BuiltinFunctions {
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

class BuiltinAlloc : public BuiltinFunction {
public: /* Methods: */
    BuiltinAlloc (unsigned size) : m_size (size) { }
    ~BuiltinAlloc () { }
    
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
class BuiltinVArith : public BuiltinFunction {
public: /* Methods: */
    BuiltinVArith (const SecreC::Imop* imop) : m_imop (imop) { }
    ~BuiltinVArith () { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinVArith (m_imop);
    }

private: /* Fields: */
    const SecreC::Imop* const m_imop;
};

/*******************************************************************************
  BuiltinVCast
*******************************************************************************/

class BuiltinVCast : public BuiltinFunction {
public: /* Methods: */
    BuiltinVCast (VMDataType dest, VMDataType src)
        : m_dest (dest)
        , m_src (src)
    { }

    ~BuiltinVCast () { }

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

class BuiltinVBoolCast : public BuiltinFunction {
public: /* Methods: */
    BuiltinVBoolCast (VMDataType src)
        : m_src (src)
    { }

    ~BuiltinVBoolCast () { }

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

class BuiltinStrAppend : public BuiltinFunction {
public: /* Methods: */
    BuiltinStrAppend () { }
    ~BuiltinStrAppend () { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinStrAppend ();
    }
};

/*******************************************************************************
  BuiltinStrDup
*******************************************************************************/

class BuiltinStrDup : public BuiltinFunction  {
public: /* Methods: */
    BuiltinStrDup () {}
    ~BuiltinStrDup () { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinStrDup ();
    }
};

/*******************************************************************************
  BuiltinNumericToString
*******************************************************************************/

class BuiltinNumericToString : public BuiltinFunction  {
public: /* Methods: */
    explicit BuiltinNumericToString (StringLiterals* strLit, unsigned base = 10)
        : m_strLit (strLit)
        , m_base (base)
    { }

    ~BuiltinNumericToString () { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinNumericToString (m_strLit, m_base);
    }

private: /* Fields: */
    StringLiterals*  const m_strLit;
    unsigned         const m_base;
};

/*******************************************************************************
  BuiltinBoolToString
*******************************************************************************/

class BuiltinBoolToString : public BuiltinFunction  {
public: /* Methods: */
    explicit BuiltinBoolToString (StringLiterals* strLit)
        : m_strLit (strLit)
    { }

    ~BuiltinBoolToString () { }

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

class BuiltinStringCmp : public BuiltinFunction  {
public: /* Methods: */

    BuiltinStringCmp () { }

    ~BuiltinStringCmp () { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinStringCmp ();
    }
};

} // namespace SecreCC

#endif
