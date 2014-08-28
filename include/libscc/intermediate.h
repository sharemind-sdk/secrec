#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include "blocks.h"
#include "context.h"
#include "imop.h"
#include "log.h"
#include "ModuleMap.h"
#include "symboltable.h"

#include <boost/optional/optional_fwd.hpp>
#include <iosfwd>
#include <string>

namespace SecreC {

class TreeNodeModule;
class StringTable;

class ICode {
    ICode (const ICode&); // do not implement
    ICode& operator = (const ICode&); // do not implement
public: /* Types: */

    enum Status { NOT_READY, OK, ERROR };

public: /* Methods: */
    ICode ()
        : m_status (NOT_READY)
        , m_modules (m_context)
    { }


    TreeNodeModule* parseMain (const boost::optional<std::string>& mfile);
    void compile (TreeNodeModule *mod);

    SymbolTable& symbols () { return m_symbols; }
    const SymbolTable& symbols () const { return m_symbols; }
    Program& program () { return m_program; }
    const Program& program () const { return m_program; }
    Status status () const { return m_status; }
    CompileLog& compileLog () { return m_log; }
    const CompileLog& compileLog () const { return m_log; }
    ModuleMap& modules () { return m_modules; }
    Context& context () { return m_context; }
    StringTable& stringTable ();

private: /* Fields: */

    Status          m_status;
    Context         m_context;
    SymbolTable     m_symbols;
    ModuleMap       m_modules;
    Program         m_program;
    TreeNodeModule* m_ast;
    CompileLog      m_log;
};

std::ostream &operator<<(std::ostream &out, const ICode::Status &s);
std::ostream &operator<<(std::ostream &out, const ICodeList &c);
std::ostream &operator<<(std::ostream &out, const ICode &icode);

} // namespace SecreC

#endif // INTERMEDIATE_H
