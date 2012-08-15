/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef VM_CODE_H
#define VM_CODE_H

#include "VMInstruction.h"

#include <list>
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
class VMBlock {
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
class VMFunction {
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

    ~VMFunction () { }

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

class VMBinding {
public: /* Methods: */

    VMBinding (VMLabel* label, std::string name)
        : m_label (label)
        , m_name (name)
    { }

    ~VMBinding () { }

    friend std::ostream& operator << (std::ostream& os, const VMBinding& binding);

private: /* Fields: */

    VMLabel*    const m_label;
    std::string const m_name;
};

/*******************************************************************************
  VMSection
*******************************************************************************/

class VMSection {
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

class VMBindingSection : public VMSection {
private: /* Types: */
    typedef std::list<VMBinding > BindingList;
    typedef BindingList::iterator iterator;
    typedef BindingList::const_iterator const_iterator;

public: /* Methods: */


    explicit VMBindingSection (const char* name)
        : VMSection (name)
    { }

    virtual ~VMBindingSection () { }

    void addBinding (VMLabel* label, const std::string& name) {
        m_bindings.push_back (VMBinding (label, name));
    }

    iterator begin () { return m_bindings.begin (); }
    iterator end () { return m_bindings.end (); }
    const_iterator begin () const { return m_bindings.begin (); }
    const_iterator end () const { return m_bindings.end (); }

protected:

    std::ostream& printBodyV (std::ostream& os) const;

private: /* Fields: */

    BindingList m_bindings;
};

/*******************************************************************************
  VMDataSection
*******************************************************************************/

class VMDataSection : public VMSection {
private:
    VMDataSection (const VMDataSection&); // DO NOT IMPLEMENT
    void operator = (const VMDataSection&); // DO NOT IMPLEMENT
public: /* Types: */

    enum Type {
        DATA = 0,
        RODATA
    };

    struct Record {
    private:
        void operator = (const Record&); // DO NOT IMPLEMENT
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
        , m_type (type)
    { }

    virtual ~VMDataSection () { }

    void addRecord (VMLabel* l, const char* t, const std::string& v) {
        m_records.push_back (Record (l, t, v));
    }

protected:

    std::ostream& printBodyV (std::ostream& os) const;

private: /* Fields: */

    const Type  m_type;
    Records     m_records;
};


/*******************************************************************************
  VMCodeSection
*******************************************************************************/

class VMCodeSection : public VMSection {
public: /* Types: */

    typedef std::list<VMFunction > FunctionList;
    typedef FunctionList::iterator iterator;
    typedef FunctionList::const_iterator const_iterator;

public: /* Methods: */

    VMCodeSection ()
        : VMSection ("TEXT")
        , m_numGlobals (0)
    { }

    ~VMCodeSection () { }

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

    std::ostream& printBodyV (std::ostream& os) const;

private: /* Fields: */

    FunctionList    m_functions;
    unsigned        m_numGlobals;
};

/*******************************************************************************
  VMLinkingUnit
*******************************************************************************/

class VMLinkingUnit {
private:

    VMLinkingUnit (const VMLinkingUnit&); // DO NOT IMPLEMENT
    void operator = (const VMLinkingUnit&); // DO NOT IMPPLEMENT

public: /* Types: */

    typedef std::list<VMSection* > SectionList;
    typedef SectionList::iterator iterator;
    typedef SectionList::const_iterator const_iterator;

public: /* Methods: */

    VMLinkingUnit () { }
    ~VMLinkingUnit ();

    /// Take ownership of the section.
    /// The section will be released on destructions.
    void addSection (VMSection* section);

    friend std::ostream&
    operator << (std::ostream& os, const VMLinkingUnit& code);

private: /* Fields: */

    SectionList m_sections;
};

std::ostream& operator << (std::ostream& os, const VMBlock& block);
std::ostream& operator << (std::ostream& os, const VMFunction& function);
std::ostream& operator << (std::ostream& os, const VMBinding& binding);
std::ostream& operator << (std::ostream& os, const VMSection& section);
std::ostream& operator << (std::ostream& os, const VMLinkingUnit& code);
std::ostream& operator << (std::ostream& os, const VMDataSection::Record& record);



} // namespace SecreCC

#endif
