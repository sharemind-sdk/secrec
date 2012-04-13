/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef STRING_LITERALS_H
#define STRING_LITERALS_H

#include <map>

#include <libscc/constant.h>

namespace SecreCC {

class VMLabel;
class VMSymbolTable;
class VMDataSection;

/*******************************************************************************
  StringLiterals
*******************************************************************************/

class StringLiterals {
private:
    StringLiterals (const StringLiterals&); // DO NOT IMPLEMENT
    void operator = (const StringLiterals&); // DO NOT IMPLEMENT

public:  /* Types: */

    struct LiteralInfo {
        VMLabel* label;
        size_t   size;

        LiteralInfo (VMLabel* l, size_t s)
            : label (l), size (s)
        { }
    };

private:

    typedef std::map<const SecreC::ConstantString*, LiteralInfo> LitMap;

public: /* Methods: */

    StringLiterals ();
    ~StringLiterals ();

    void init (VMSymbolTable& st, VMDataSection* section);
    size_t getSize (const SecreC::ConstantString* str) const;
    VMLabel* getLabel (const SecreC::ConstantString* str) const;
    LiteralInfo insert (const SecreC::ConstantString* str);

private: /* Fields: */

    VMSymbolTable*   m_st;
    VMDataSection*   m_dataSection;
    LitMap           m_literals;
};

} // namespace SecreCC

#endif // STRING_LITERALS_H
