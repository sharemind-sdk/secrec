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

#ifndef STRING_LITERALS_H
#define STRING_LITERALS_H

#include <map>

#include <libscc/Constant.h>
#include <memory>


namespace SecreCC {

class VMLabel;
class VMSymbolTable;
class VMDataSection;

/*******************************************************************************
  StringLiterals
*******************************************************************************/

class __attribute__ ((visibility("internal"))) StringLiterals {
private:
    StringLiterals (const StringLiterals&) = delete;
    StringLiterals& operator = (const StringLiterals&) = delete;

public:  /* Types: */

    struct LiteralInfo {
        VMLabel* label;
        size_t   size;

        LiteralInfo (VMLabel* l, size_t s)
            : label (l), size (s)
        { }
    };

private:

    typedef std::map<std::string, const LiteralInfo> LitMap;

public: /* Methods: */

    StringLiterals(VMSymbolTable & st);
    ~StringLiterals ();

    void init(std::shared_ptr<VMDataSection> section);
    LiteralInfo insert (const SecreC::ConstantString* str, bool asNullTerminated = true);
    LiteralInfo insert (const std::string& str, bool asNullTerminated = true);

private: /* Fields: */

    VMSymbolTable & m_st;
    std::shared_ptr<VMDataSection> m_dataSection;
    LitMap           m_literals;
    size_t           m_uniq;
};

} // namespace SecreCC

#endif // STRING_LITERALS_H
