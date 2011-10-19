#ifndef BUILTIN_H
#define BUILTIN_H

#include "VMValue.h"
#include "VMCode.h"
#include "VMSymbolTable.h"

#include <map>

namespace SecreC {
    class Imop;
}

namespace SecreCC {

/*******************************************************************************
  BuiltinFunction
*******************************************************************************/

class BuiltinFunction {
public:
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
    void insert (VMLabel* label, const BuiltinFunction& func);
    void generateAll (VMCode& code, VMSymbolTable& st);

protected:

    void eraseAll ();

private: /* Fields: */

    Map m_funtions;
};

/*******************************************************************************
  BuiltinAlloc
*******************************************************************************/

class BuiltinAlloc : public BuiltinFunction {
public:
    BuiltinAlloc (unsigned size) : m_size (size) { }
    ~BuiltinAlloc () { }
    
    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinAlloc (m_size);
    }

private:
    const unsigned m_size;
};

/*******************************************************************************
  BuiltinVArith
*******************************************************************************/

/// Builtin vectorised arithmetic operations
class BuiltinVArith : public BuiltinFunction {
public:
    BuiltinVArith (const SecreC::Imop* imop) : m_imop (imop) { }
    ~BuiltinVArith () { }

    void generate (VMFunction& function, VMSymbolTable& st);

    BuiltinFunction* clone () const {
        return new BuiltinVArith (m_imop);
    }

private:
    const SecreC::Imop* const m_imop;
};


} // namespace SecreCC

#endif
