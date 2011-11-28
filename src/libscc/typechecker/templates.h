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
#include <utility>
#include <map>
#include <boost/foreach.hpp>

#include "types.h"
#include "treenode.h"

namespace SecreC {

/*******************************************************************************
  Instantiation
*******************************************************************************/

/// This class acts as index for template instances. Nameley a template instance
/// is defined by it's declaration and the parameters it takes.
class Instantiation {
public: /* Types: */

    typedef SecurityType* TemplateParam;
    typedef std::vector<TemplateParam> TemplateParams;
    typedef TemplateParams::iterator iterator;
    typedef TemplateParams::const_iterator const_iterator;

public: /* Methods: */

    explicit Instantiation (SymbolTemplate* templ)
        : m_templ (templ)
    { }

    SymbolTemplate* getTemplate () const {
        return m_templ;
    }

    const_iterator begin () const { return m_params.begin (); }
    const_iterator end () const { return m_params.end (); }
    iterator begin () { return m_params.begin (); }
    iterator end () { return m_params.end (); }

    void addParam (SecurityType* secTy) {
        m_params.push_back (secTy);
    }

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
    TreeNodeProcDef* m_generatedBody;
    SymbolTable*     m_localSymbolTable;
    InstanceInfo ()
        : m_generatedBody (0)
        , m_localSymbolTable (0)
    { }
};

class TemplateInstantiator {
public: /* Methods: */
    TemplateInstantiator () { }
    ~TemplateInstantiator () { }

    /**
     * Marks instantiation for future code generation. Sets local symbol table
     * and binds template parameters in local symbol table to proper security types.
     */
    TreeNodeProcDef* add (const Instantiation& i, SymbolTable* st);

    /**
     * Gets template instance for code generation.
     * \retval false  if no more templates need to be instansiated
     * \retval true   more templates need to be instansiated
     * \param[out] proc is set to body of procedure only if \a true is returned
     * \param[out] st is set to local symbol table only if \a true is returned
     */
    bool getForInstantiation (TreeNodeProcDef*& proc, SymbolTable*& st);

    inline bool isInstantiated (const Instantiation& i) const {
        return m_generated.find (i) != m_generated.end ();
    }

    SymbolTable* getLocalST (const Instantiation& i) const {
        std::cerr << "getLocalST:" << i << std::endl;
        return at (i).m_localSymbolTable;
    }

protected:

    const InstanceInfo& at (const Instantiation& i) const {
        assert (m_instanceInfo.find (i) != m_instanceInfo.end ());
        return m_instanceInfo.find (i)->second;
    }

private: /* Fields: */

    std::set<Instantiation>                 m_generated;    // set of generated instances
    std::map<Instantiation, InstanceInfo >  m_instanceInfo;
    std::list<Instantiation >               m_workList;
};


} // namespace SecreC;

#endif
