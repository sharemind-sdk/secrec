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

#ifndef VM_VALUE_H
#define VM_VALUE_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include "OStreamable.h"


namespace SecreCC {

/******************************************************************
  VMValue
******************************************************************/

/**
 * Representation of Sharemind virtual machine values.
 * Only (de)allocated by the VM symbol table class.
 */
class __attribute__ ((visibility("internal"))) VMValue {

protected: /* Methods: */

    VMValue(std::shared_ptr<OStreamable> streamable)
        : m_streamable(std::move(streamable))
    {}

public: /* Methods: */

    virtual ~VMValue();

    std::shared_ptr<OStreamable> const & streamable() const noexcept
    { return m_streamable; }

private: /* Fields: */

    std::shared_ptr<OStreamable> m_streamable;

};

/******************************************************************
  VMImm
******************************************************************/

class __attribute__ ((visibility("internal"))) VMImm final: public VMValue {
public: /* Methods: */

    explicit VMImm(std::uint64_t value);

};

/******************************************************************
  VMStack
******************************************************************/

class __attribute__ ((visibility("internal"))) VMStack final: public VMValue {
public: /* Methods: */

    explicit VMStack(unsigned number);

};

/******************************************************************
  VMReg
******************************************************************/

class __attribute__ ((visibility("internal"))) VMReg final: public VMValue {
public: /* Methods: */

    explicit VMReg(unsigned number);

};

/******************************************************************
  VMLabel
******************************************************************/

class __attribute__ ((visibility("internal"))) VMLabel final: public VMValue {
public: /* Methods: */

    explicit VMLabel(std::string name);

    std::shared_ptr<OStreamable> nameStreamable() const noexcept;
    std::string const & name() const noexcept;

};

/******************************************************************
 VMVReg
******************************************************************/

/**
 * Virtual register.
 * Representation of unallocated registers. Will be set to concrete registers
 * at the register allocation pass.
 */
class __attribute__ ((visibility("internal"))) VMVReg final: public VMValue {
public: /* Methods: */

    explicit VMVReg(bool const isGlobal);

    bool isGlobal() const noexcept { return m_isGlobal; }
    void setActualReg(VMValue const & reg);

private: /* Fields: */

    bool m_isGlobal;
    std::string m_value;

};

} // namespace SecreCC

#endif
