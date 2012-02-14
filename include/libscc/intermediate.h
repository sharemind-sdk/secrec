#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include <ostream>

#include "blocks.h"
#include "imop.h"
#include "log.h"
#include "symboltable.h"
#include "ModuleMap.h"

namespace SecreC {

class TreeNodeModule;
class Context;

class ICode {
private:
    ICode (const ICode&); // do not implement
    ICode& operator = (const ICode&); // do not implement
public: /* Types: */
    enum Status { NOT_READY, OK, E_NOT_IMPLEMENTED, E_EMPTY_PROGRAM,
                  E_NO_MAIN, E_TYPE, E_OTHER, E_NO_MEM };

public: /* Methods: */
    ICode ()
        : m_status (NOT_READY)
    {}

    Status init (Context& cxt, TreeNodeModule* mod);

    SymbolTable& symbols () { return m_symbols; }
    const SymbolTable& symbols () const { return m_symbols; }
    Program& program () { return m_program; }
    const Program& program () const { return m_program; }
    Status status () const { return m_status; }
    CompileLog& compileLog () { return m_log; }
    const CompileLog& compileLog () const { return m_log; }
    ModuleMap& modules () { return m_modules; }

private: /* Fields: */

    SymbolTable  m_symbols;
    ModuleMap    m_modules;
    Program      m_program;
    Status       m_status;
    CompileLog   m_log;
};


} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::ICode::Status &s);
std::ostream &operator<<(std::ostream &out, const SecreC::ICodeList &c);
std::ostream &operator<<(std::ostream &out, const SecreC::ICode &icode);

#endif // INTERMEDIATE_H
