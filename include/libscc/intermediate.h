#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include <set>
#include <sstream>
#include "blocks.h"
#include "imop.h"
#include "log.h"
#include "symboltable.h"
#include "types.h"


namespace SecreC {

class TreeNodeProgram;
class VirtualMachine;

class ICode {
    public: /* Types: */
        enum Status { NOT_READY, OK, E_NOT_IMPLEMENTED, E_EMPTY_PROGRAM,
                      E_NO_MAIN, E_TYPE, E_OTHER, E_NO_MEM };

    public: /* Methods: */
        inline ICode()
            : m_status(NOT_READY) {}

        Status init(TreeNodeProgram *p);

        const SymbolTable &symbols() const { return m_symbols; }
        const ICodeList &code() const { return m_code; }
        Status status() const { return m_status; }
        const CompileLog &compileLog() const { return m_log; }

        void run (VirtualMachine&) const;

    private: /* Fields: */
        SymbolTable m_symbols;
        ICodeList   m_code;
        Status      m_status;
        CompileLog  m_log;
};


} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::ICode::Status &s);
std::ostream &operator<<(std::ostream &out, const SecreC::ICodeList &c);
std::ostream &operator<<(std::ostream &out, const SecreC::ICode &icode);

#endif // INTERMEDIATE_H
