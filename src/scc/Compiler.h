#ifndef CODEGEN_H
#define CODEGEN_H

#include "VMCode.h"
#include "VMSymbolTable.h"

#include <libscc/intermediate.h>
#include <libscc/blocks.h>


namespace SecreCC {

/*******************************************************************************
  Compiler
*******************************************************************************/

class Compiler {
private:

    Compiler (const Compiler&); // DO NOT IMPLEMENT
    void operator = (const Compiler&); // DO NOT IMPLEMENT

public: /* Methods: */

    Compiler (const SecreC::ICode& code);
    ~Compiler ();

    void run ();

    VMCode& target () { return m_target; }
    VMSymbolTable& st () { return m_st; }

protected:

    void cgProcedure (const SecreC::SymbolProcedure* proc, 
                      const std::list<SecreC::Block*>& blocks);
    void cgBlock (VMFunction& func, const SecreC::Block* block);
    void cgImop (VMBlock& block, const SecreC::Imop* imop);

    void cgJump (VMBlock& block, const SecreC::Imop* imop);
    void cgAssign (VMBlock& block, const SecreC::Imop* imop);
    void cgCall (VMBlock& block, const SecreC::Imop* imop);
    void cgParam (VMBlock& block, const SecreC::Imop* imop);
    void cgReturn (VMBlock& block, const SecreC::Imop* imop);
    void cgArithm (VMBlock& block, const SecreC::Imop* imop);

private: /* Fields: */

    const SecreC::ICode&  m_code;    ///< SecreC intermediate code
    SecreC::Blocks        m_cfg;     ///< Control flow graph
    VMCode                m_target;  ///< Target code
    VMSymbolTable         m_st;      ///< VM symbol table
    unsigned              m_uniq;    ///< Unique count, for generating fresh labels, and registers
    unsigned              m_param;   ///< Current param count
};

} // namespace SecreCC

#endif
