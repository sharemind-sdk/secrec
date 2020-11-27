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

#ifndef VM_SYMBOL_TABLE_H
#define VM_SYMBOL_TABLE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <sharemind/Concat.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


namespace SecreC { class Symbol; }
namespace SecreCC {

class VMValue;
class VMImm;
class VMReg;
class VMStack;
class VMLabel;
class VMVReg;

class __attribute__ ((visibility("internal"))) VMSymbolTable {

public: /* Methods: */

    VMSymbolTable();
    VMSymbolTable(VMSymbolTable const &) = delete;
    VMSymbolTable(VMSymbolTable &&) = delete;

    ~VMSymbolTable();

    VMSymbolTable & operator=(VMSymbolTable const &) = delete;
    VMSymbolTable & operator=(VMSymbolTable &&) = delete;

    VMValue * find(SecreC::Symbol const * const symbol) const noexcept;
    bool store(SecreC::Symbol const * const symbol, VMValue * const);

    VMLabel * getUniqLabel();

    template <typename ... Args>
    VMLabel * getUniqLabel(Args && ... args) {
        return getLabel(sharemind::concat(std::forward<Args>(args)...,
                                          m_uniq++));
    }

    VMImm * getImm(std::uint64_t const value);
    VMReg * getReg(std::size_t const number);
    VMStack * getStack(std::size_t const number);
    VMLabel * getLabel(std::string const & name);
    VMVReg * getVReg(bool const isGlobal);

public: /* Fields: */

    std::unordered_map<std::size_t, std::unique_ptr<VMReg>> m_globals;
    std::unordered_map<std::size_t, std::unique_ptr<VMStack>> m_locals;
    std::unordered_map<std::uint64_t, std::unique_ptr<VMImm>> m_imms;
    std::unordered_map<std::string, std::unique_ptr<VMLabel>> m_labels;
    std::vector<std::unique_ptr<VMVReg> > m_vregs;
    std::unordered_map<SecreC::Symbol const *, VMValue *> m_mapping;
    std::size_t m_uniq = 0u;

}; /* class VMSymbolTable { */

} /* namespace SecreCC { */

#endif
