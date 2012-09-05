/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_TEMPLATES_H
#define SECREC_TEMPLATES_H

#include <vector>
#include <set>
#include <list>
#include <utility>
#include <map>
#include <boost/foreach.hpp>

#include "symbol.h"
#include "types.h"
#include "treenode_fwd.h"

/**
 * This file contains functionality for template instantiation
 * and for tracking, via the TemplateInstantiator class, which
 * templates have been instantiated.
 */

namespace SecreC {

class SymbolTable;
class ModuleInfo;
typedef SecurityType* TemplateParam;
typedef std::vector<TemplateParam> TemplateParams;

/*******************************************************************************
  Instantiation
*******************************************************************************/

/**
 * This class acts as index for template instances. Nameley a template
 * instance is defined by it's declaration and the parameters it takes.
 */
class Instantiation {
public: /* Types: */

    typedef TemplateParams::iterator iterator;
    typedef TemplateParams::const_iterator const_iterator;

public: /* Methods: */

    Instantiation ()
        : m_templ (0)
    { }

    explicit Instantiation (SymbolTemplate* templ)
        : m_templ (templ)
    { }

    inline void swap (Instantiation& i) {
        std::swap (m_templ, i.m_templ);
        m_params.swap (i.m_params);
    }

    SymbolTemplate* getTemplate () const { return m_templ; }

    const_iterator begin () const { return m_params.begin (); }
    const_iterator end () const { return m_params.end (); }
    iterator begin () { return m_params.begin (); }
    iterator end () { return m_params.end (); }

    void addParam (SecurityType* secTy) { m_params.push_back (secTy); }

    unsigned templateParamCount () const { return m_params.size (); }
    unsigned unrestrictedTemplateParamCount () const;
    unsigned quantifiedDomainOccurrenceCount () const;

    friend bool operator == (const Instantiation& a, const Instantiation& b);
    friend bool operator <  (const Instantiation& a, const Instantiation& b);
    friend std::ostream& operator << (std::ostream& os, const Instantiation& i);

protected:

    inline std::pair<SymbolTemplate*, TemplateParams> toPair () const {
        return std::make_pair (m_templ, m_params);
    }

private: /* Fields: */

    SymbolTemplate*  m_templ;
    TemplateParams   m_params;
};

inline bool operator == (const Instantiation& a, const Instantiation& b) {
    return a.toPair () == b.toPair ();
}

inline bool operator < (const Instantiation& a, const Instantiation& b) {
    return a.toPair () < b.toPair ();
}

inline std::ostream& operator << (std::ostream& os, const Instantiation& i) {
    os << i.getTemplate () << "(";
    BOOST_FOREACH (SecurityType* ty, i) {
        if (ty != *i.begin ()) os << ',';
        os << ty;
    }

    os << ')';
    return os;
}


/*******************************************************************************
  TemplateInstantiator
*******************************************************************************/

struct InstanceInfo {
    TreeNodeProcDef*  m_generatedBody;
    SymbolTable*      m_localScope;
    ModuleInfo*       m_moduleInfo;

    InstanceInfo ()
        : m_generatedBody (0)
        , m_localScope (0)
        , m_moduleInfo (0)
    { }
};

class TemplateInstantiator {

private: /* Types: */

    typedef std::map<Instantiation, InstanceInfo> InstanceInfoMap;

public: /* Methods: */

    ~TemplateInstantiator();

    /**
     * Marks instantiation for future code generation. Sets local symbol table
     * and binds template parameters in local symbol table to proper security types.
     */
    const InstanceInfo& add (const Instantiation& i, ModuleInfo& mod);

    /**
     * Gets template instance for code generation.
     * \retval false  if no more templates need to be instansiated
     * \retval true   more templates need to be instansiated
     * \param[out] info required instantiation information
     */
    bool getForInstantiation (InstanceInfo& info);

    inline bool isInstantiated (const Instantiation& i) const {
        return m_generated.find (i) != m_generated.end ();
    }

    SymbolTable* symbolTable (const Instantiation& i) const {
        return at (i).m_localScope;
    }

    ModuleInfo* getModule (const Instantiation& i) const {
        return at (i).m_moduleInfo;
    }

    const InstanceInfo& at (const Instantiation& i) const {
        assert (m_instanceInfo.find (i) != m_instanceInfo.end ());
        return m_instanceInfo.find (i)->second;
    }

private: /* Fields: */

    std::set<Instantiation>   m_generated;    // set of generated instances
    InstanceInfoMap           m_instanceInfo;
    std::list<Instantiation > m_workList;
};


} // namespace SecreC;

#endif
