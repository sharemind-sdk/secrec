/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef REGISTER_ALLOCATOR_H
#define REGISTER_ALLOCATOR_H

#include <set>
#include <stack>
#include <memory>

namespace SecreC {
    class Imop;
    class LiveVariables;
    class Symbol;
}

namespace SecreCC {

class VMSymbolTable;
class VMCode;
class VMValue;
class VMVReg;
class VMBlock;
class VMFunction;

/*******************************************************************************
  RegisterAllocator
*******************************************************************************/

class RegisterAllocator {
public: /* Types: */

    typedef std::auto_ptr<SecreC::LiveVariables > LVPtr;
    typedef std::set<const SecreC::Symbol*> Symbols;
    typedef std::set<VMVReg*> RegSet;
    typedef std::stack<VMVReg*> RegStack;

public: /* Methods: */

    RegisterAllocator ();
    ~RegisterAllocator ();

    void init (VMSymbolTable& st, LVPtr lv);

    VMVReg* temporaryReg ();

    void enterFunction (VMFunction& function);
    void exitFunction (VMFunction& function);
    void enterBlock (VMBlock& block);
    void exitBlock (VMBlock& block);

    unsigned globalCount ();

    void getReg (const SecreC::Imop& imop);

protected:

    void defSymbol (const SecreC::Symbol* symbol);

private: /* Fields: */

    class InferenceGraph;

    VMSymbolTable*          m_st;
    LVPtr                   m_lv; ///< Pointer to live variables.
    InferenceGraph*         m_inferenceGraph;
    RegSet                  m_live;
    RegStack                m_temporaries;
    bool                    m_isGlobal;
};

} // namespace SecreCC

#endif
