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

#ifndef VM_CODE_H
#define VM_CODE_H

#include "VMInstruction.h"

#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include "OStreamable.h"
#include "VMDataType.h"


namespace SecreC {
    class Block;
} // namespace SecreC

namespace SecreCC {

/*******************************************************************************
  VMBlock
*******************************************************************************/

/**
 * Basic block of VM code instructions.
 */
class __attribute__ ((visibility("internal"))) VMBlock {
public: /* Types: */

    typedef std::list<VMInstruction > InstList;
    typedef InstList::iterator iterator;
    typedef InstList::const_iterator const_iterator;

public: /* Methods: */

    VMBlock(std::shared_ptr<OStreamable> name,
            SecreC::Block const * block = nullptr) noexcept
        : m_name(std::move(name))
        , m_secrecBlock (block)
    {}

    VMBlock(SecreC::Block const * block = nullptr) noexcept
        : m_secrecBlock(block)
    {}

    const SecreC::Block* secrecBlock () const { return m_secrecBlock; }

    iterator begin () { return m_instructions.begin (); }
    iterator end () { return m_instructions.end (); }
    const_iterator begin () const { return m_instructions.begin (); }
    const_iterator end () const { return m_instructions.end (); }
    void push_back (const VMInstruction& i) {
        m_instructions.push_back (i);
    }

    VMInstruction& push_new (void) {
        m_instructions.push_back (VMInstruction ());
        return m_instructions.back ();
    }

    friend std::ostream& operator << (std::ostream& os, const VMBlock& block);

private: /* Fields: */

    std::shared_ptr<OStreamable> const m_name;
    const SecreC::Block*  const  m_secrecBlock;
    InstList                     m_instructions;
};

/*******************************************************************************
  VMFunction
*******************************************************************************/

/**
 * Representation of SecreC function in the VM
 */
class __attribute__ ((visibility("internal"))) VMFunction {
public: /* Types: */

    typedef std::list<VMBlock > BlockList;
    typedef BlockList::iterator iterator;
    typedef BlockList::const_iterator const_iterator;

public: /* Methods: */

    explicit VMFunction(std::shared_ptr<OStreamable> name)
        : m_name(std::move(name))
        , m_isStart (false)
        , m_numLocals (0)
    {}

    unsigned numLocals () const { return m_numLocals; }
    void setNumLocals (unsigned n) { m_numLocals = n; }
    void setIsStart () { m_isStart = true; }
    bool isStart () const { return m_isStart; }

    iterator begin () { return m_blocks.begin (); }
    iterator end () { return m_blocks.end (); }
    const_iterator begin () const { return m_blocks.begin (); }
    const_iterator end () const { return m_blocks.end (); }
    VMFunction& push_back (const VMBlock& b) {
        m_blocks.push_back (b);
        return *this;
    }

    friend std::ostream& operator << (std::ostream& os, const VMFunction& function);

private: /* Fields: */

   BlockList       m_blocks;  ///< VM blocks that define this function.
   std::shared_ptr<OStreamable> const m_name; ///< Name of the function.
   bool            m_isStart; ///< Special function to mark the start of the byte code.
   unsigned        m_numLocals; ///< Number of local registers, either "reg" or "stack".
};


/*******************************************************************************
  VMBinding
*******************************************************************************/

class __attribute__ ((visibility("internal"))) VMBinding {
public: /* Methods: */

    VMBinding(std::shared_ptr<OStreamable> label, std::string name)
        : m_label(std::move(label))
        , m_name(std::move(name))
    { }

    friend std::ostream& operator << (std::ostream& os, const VMBinding& binding);

private: /* Fields: */

    std::shared_ptr<OStreamable> const m_label;
    std::string const m_name;
};

/*******************************************************************************
  VMSection
*******************************************************************************/

class __attribute__ ((visibility("internal"))) VMSection {
public: /* Methods: */

    VMSection (const char* name) : m_name (name) { }
    virtual ~VMSection () { }

    const char* name () const { return m_name; }
    friend std::ostream& operator << (std::ostream& os, const VMSection& section);

protected:

    virtual std::ostream& printBodyV (std::ostream& os) const = 0;

protected: /* Fields: */

    const char* const m_name;
};

/*******************************************************************************
  VMBindingSection
*******************************************************************************/

class __attribute__ ((visibility("internal"))) VMBindingSection final
    : public VMSection
{
private: /* Types: */
    typedef std::list<VMBinding > BindingList;
    typedef BindingList::iterator iterator;
    typedef BindingList::const_iterator const_iterator;

public: /* Methods: */


    explicit VMBindingSection (const char* name)
        : VMSection (name)
    { }

    void addBinding(std::shared_ptr<OStreamable> label, std::string name)
    { m_bindings.emplace_back(std::move(label), std::move(name)); }

    iterator begin () { return m_bindings.begin (); }
    iterator end () { return m_bindings.end (); }
    const_iterator begin () const { return m_bindings.begin (); }
    const_iterator end () const { return m_bindings.end (); }

protected:

    std::ostream & printBodyV(std::ostream & os) const final override;

private: /* Fields: */

    BindingList m_bindings;
};

/*******************************************************************************
  VMDataSection
*******************************************************************************/

class __attribute__ ((visibility("internal"))) VMDataSection final
    : public VMSection
{
private:
    VMDataSection (const VMDataSection&) = delete;
    VMDataSection& operator = (const VMDataSection&) = delete;
public: /* Types: */

    enum Type {
        DATA = 0,
        RODATA
    };

    struct StringRecord {
    public: /* Methods: */
        StringRecord & operator=(StringRecord const &) = delete;
        StringRecord(std::shared_ptr<OStreamable> label, std::string value)
            : m_label(std::move(label))
            , m_value(std::move(value))
        {}

        std::shared_ptr<OStreamable> const m_label;
        std::string const m_value;

        std::ostream & print(std::ostream & os) const;
    };

private:

    typedef std::list<StringRecord> Records;

public: /* Methods: */

    explicit VMDataSection (Type type)
        : VMSection (type == RODATA ? "RODATA" : "DATA")
    { }

    void addStringRecord(std::shared_ptr<OStreamable> label, std::string value)
    { m_records.emplace_back(std::move(label), std::move(value)); }

protected:

    std::ostream & printBodyV(std::ostream & os) const final override;

private: /* Fields: */

    Records     m_records;
};


/*******************************************************************************
  VMCodeSection
*******************************************************************************/

class __attribute__ ((visibility("internal"))) VMCodeSection final
    : public VMSection
{
public: /* Types: */

    typedef std::list<VMFunction > FunctionList;
    typedef FunctionList::iterator iterator;
    typedef FunctionList::const_iterator const_iterator;

public: /* Methods: */

    VMCodeSection ()
        : VMSection ("TEXT")
        , m_numGlobals (0)
    { }

    unsigned numGlobals () const { return m_numGlobals; }
    void setNumGlobals (unsigned n) { m_numGlobals = n; }
    iterator begin () { return m_functions.begin (); }
    iterator end () { return m_functions.end (); }
    const_iterator begin () const { return m_functions.begin (); }
    const_iterator end () const { return m_functions.end (); }
    void push_back (const VMFunction& f) { m_functions.push_back (f); }

    /**
     * \todo VMFunction is not that light-weight class. Should probably
     * use the intrusive boost lists again, but this really is not the
     * current bottle neck.
     */

protected:

    std::ostream & printBodyV(std::ostream & os) const final override;

private: /* Fields: */

    FunctionList    m_functions;
    unsigned        m_numGlobals;
};

/*******************************************************************************
  VMLinkingUnit
*******************************************************************************/

class __attribute__ ((visibility("internal"))) VMLinkingUnit {

public: /* Methods: */

    VMLinkingUnit();
    VMLinkingUnit(VMLinkingUnit &&) = delete;
    VMLinkingUnit(VMLinkingUnit const &) = delete;
    VMLinkingUnit & operator=(VMLinkingUnit &&) = delete;
    VMLinkingUnit & operator=(VMLinkingUnit const &) = delete;

    ~VMLinkingUnit();

    void addSection(std::shared_ptr<VMSection> section);

    friend std::ostream &
    operator<<(std::ostream & os, VMLinkingUnit const & code);

private: /* Fields: */

    std::vector<std::shared_ptr<VMSection>> m_sections;

};

std::ostream& operator << (std::ostream& os, const VMBlock& block)
        __attribute__((visibility("internal")));
std::ostream& operator << (std::ostream& os, const VMFunction& function)
        __attribute__((visibility("internal")));
std::ostream& operator << (std::ostream& os, const VMBinding& binding)
        __attribute__((visibility("internal")));
std::ostream& operator << (std::ostream& os, const VMSection& section)
        __attribute__((visibility("internal")));
std::ostream& operator << (std::ostream& os, const VMLinkingUnit& code)
        __attribute__((visibility("internal")));
std::ostream& operator << (std::ostream& os,
                           VMDataSection::StringRecord const & record)
        __attribute__((visibility("internal")));



} // namespace SecreCC

#endif
