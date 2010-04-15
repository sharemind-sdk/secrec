#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include <set>
#include <sstream>
#include "blocks.h"
#include "imop.h"
#include "symboltable.h"
#include "types.h"


namespace SecreC {

class TreeNodeProgram;

class ICode {
    public: /* Types: */
        enum Status { NOT_READY, OK, E_NOT_IMPLEMENTED, E_EMPTY_PROGRAM,
                      E_NO_MAIN, E_TYPE, E_OTHER, E_BLOCKS, E_NO_MEM };

    public: /* Methods: */
        inline ICode()
            : m_status(NOT_READY) {}

        Status init(TreeNodeProgram *p);

        const SymbolTable &symbols() const { return m_symbols; }
        const ICodeList &code() const { return m_code; }
        const Blocks &blocks() const { return m_blocks; }
        Status status() const { return m_status; }
        std::string messages() const { return m_errorStream.str(); }

    private: /* Fields: */
        SymbolTable        m_symbols;
        ICodeList          m_code;
        Blocks             m_blocks;
        Status             m_status;
        std::ostringstream m_errorStream;
};


} // namespace SecreC

std::ostream &operator<<(std::ostream &out, const SecreC::ICode::Status &s);
std::ostream &operator<<(std::ostream &out, const SecreC::ICodeList &c);
std::ostream &operator<<(std::ostream &out, const SecreC::ICode &icode);

#endif // INTERMEDIATE_H
