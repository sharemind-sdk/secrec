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

namespace SecreC {
    class Block;
} // namespace SecreC

namespace SecreCC {

class VMLabel;

/*******************************************************************************
  VMBlock
*******************************************************************************/

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

    ~VMBlock () { }

    const VMLabel* name () const { return m_name; }
    const SecreC::Block* secrecBlock () const { return m_secrecBlock; }

    iterator begin () { return m_instructions.begin (); }
    iterator end () { return m_instructions.end (); }
    const_iterator begin () const { return m_instructions.begin (); }
    const_iterator end () const { return m_instructions.end (); }
    void push_back (const VMInstruction& i) {
        m_instructions.push_back (i);
    }

    friend std::ostream& operator << (std::ostream& os, const VMBlock& block);

private: /* Fields: */

    const VMLabel*        m_name;
    const SecreC::Block*  m_secrecBlock;
    InstList              m_instructions;
};

/*******************************************************************************
  VMFunction
*******************************************************************************/

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
    void push_back (const VMBlock& b) {
        m_blocks.push_back (b);
    }

    friend std::ostream& operator << (std::ostream& os, const VMFunction& function);

private: /* Fields: */

   BlockList       m_blocks;
   const VMLabel*  m_name;
   bool            m_isStart;
   unsigned        m_numLocals;
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

    friend std::ostream& operator << (std::ostream& os, const VMLinkingUnit& code);

private: /* Fields: */

    SectionList m_sections;
};

std::ostream& operator << (std::ostream& os, const VMBlock& block);
std::ostream& operator << (std::ostream& os, const VMFunction& function);
std::ostream& operator << (std::ostream& os, const VMBinding& binding);
std::ostream& operator << (std::ostream& os, const VMSection& section);
std::ostream& operator << (std::ostream& os, const VMLinkingUnit& code);


} // namespace SecreCC

#endif
