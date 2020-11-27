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

#include "VMDataType.h"

namespace SecreC {
    class Block;
} // namespace SecreC

namespace SecreCC {

class VMLabel;

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

    VMBlock (const VMLabel* name, const SecreC::Block* block)
        : m_name (name)
        , m_secrecBlock (block)
    { }

    const VMLabel* name () const { return m_name; }
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

    const VMLabel*        const  m_name;
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

    explicit VMFunction (const VMLabel* name)
        : m_name (name)
        , m_isStart (false)
        , m_numLocals (0)
    { }

    const VMLabel* name () const { return m_name; }

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
   const VMLabel*  m_name; ///< Name of the function.
   bool            m_isStart; ///< Special function to mark the start of the byte code.
   unsigned        m_numLocals; ///< Number of local registers, either "reg" or "stack".
};


/*******************************************************************************
  VMBinding
*******************************************************************************/

class __attribute__ ((visibility("internal"))) VMBinding {
public: /* Methods: */

    VMBinding (VMLabel* label, std::string name)
        : m_label (label)
        , m_name (name)
    { }

    friend std::ostream& operator << (std::ostream& os, const VMBinding& binding);

private: /* Fields: */

    VMLabel*    const m_label;
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

    void addBinding (VMLabel* label, const std::string& name) {
        m_bindings.push_back (VMBinding (label, name));
    }

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

    struct Record {
    private:
        Record& operator = (const Record&) = delete;
    public:
        Record (VMLabel* label, const char* type, const std::string& value)
            : m_label (label)
            , m_dataType (type)
            , m_value (value)
        { }

        VMLabel*      const  m_label;
        const char*   const  m_dataType;
        std::string   const  m_value;

        std::ostream& print (std::ostream& os) const;
    };

private:

    typedef std::list<Record > Records;

public: /* Methods: */

    explicit VMDataSection (Type type)
        : VMSection (type == RODATA ? "RODATA" : "DATA")
    { }

    void addRecord (VMLabel* l, const char* t, const std::string& v) {
        m_records.push_back (Record (l, t, v));
    }

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
                           const VMDataSection::Record& record)
        __attribute__((visibility("internal")));



} // namespace SecreCC

#endif
