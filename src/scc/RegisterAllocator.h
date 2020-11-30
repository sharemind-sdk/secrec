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

#ifndef REGISTER_ALLOCATOR_H
#define REGISTER_ALLOCATOR_H

#include <set>
#include <stack>
#include <memory>
#include <vector>

namespace SecreC {
    class Block;
    class Imop;
    class LiveVariables;
    class Symbol;
} /* namespace SecreC { */

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

class __attribute__ ((visibility("internal"))) RegisterAllocator {
public: /* Types: */

    typedef std::set<const SecreC::Symbol*> Symbols;
    typedef std::set<VMVReg*> RegSet;
    typedef std::vector<VMVReg*> RegStack;

public: /* Methods: */

    RegisterAllocator(VMSymbolTable & st);
    ~RegisterAllocator ();

    void init(std::unique_ptr<SecreC::LiveVariables> lv);

    VMVReg* temporaryReg ();

    void enterFunction (VMFunction& function);
    void exitFunction (VMFunction& function);
    void enterBlock(SecreC::Block const & secrecBlock);
    void exitBlock (VMBlock&);

    unsigned globalCount ();

    void getReg (const SecreC::Imop& imop);

protected:

    void defSymbol (const SecreC::Symbol* symbol);

private: /* Fields: */

    class InferenceGraph;

    VMSymbolTable &         m_st;
    std::unique_ptr<SecreC::LiveVariables> m_lv; ///< Pointer to live variables.
    std::unique_ptr<InferenceGraph> m_inferenceGraph;
    RegSet                  m_live;
    RegStack                m_temporaries;
    bool                    m_isGlobal;
};

} // namespace SecreCC

#endif
