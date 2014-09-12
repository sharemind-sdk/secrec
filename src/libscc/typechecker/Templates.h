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

#include <cassert>
#include <deque>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "ParserEnums.h"
#include "StringRef.h"
#include "TreeNodeFwd.h"
#include "DataType.h"
#include "TypeArgument.h"

/**
 * This file contains functionality for template instantiation
 * and for tracking, via the TemplateInstantiator class, which
 * templates have been instantiated.
 */

namespace SecreC {

class SymbolTable;
class ModuleInfo;
class SymbolTemplate;
class TypeNonVoid;
class DataTypeProcedureVoid;
class SecurityType;
class Symbol;
class SymbolTypeVariable;


typedef std::map<StringRef, TypeArgument, StringRef::FastCmp>
    TemplateVarMap;

/*******************************************************************************
  Instantiation
*******************************************************************************/

/**
 * This class acts as index for template instances. Namely, a template
 * instance is defined by its declaration and the parameters it takes.
 */
class Instantiation {
public: /* Methods: */

    explicit Instantiation (SymbolTemplate* templ)
        : m_templ (templ)
    { }

    inline void swap (Instantiation& i) {
        std::swap (m_templ, i.m_templ);
        std::swap (m_params, i.m_params);
    }

    SymbolTemplate* getTemplate () const { return m_templ; }
    std::vector<TypeArgument>& getParams () { return m_params; }
    const std::vector<TypeArgument>& getParams () const { return m_params; }

    friend bool operator == (const Instantiation& a, const Instantiation& b);
    friend bool operator <  (const Instantiation& a, const Instantiation& b);

private: /* Fields: */
    SymbolTemplate*           m_templ;
    std::vector<TypeArgument> m_params;
};

inline bool operator == (const Instantiation& a, const Instantiation& b) {
    return a.m_templ == b.m_templ && a.m_params == b.m_params;
}

inline bool operator < (const Instantiation& a, const Instantiation& b) {
    return a.m_templ < b.m_templ || (a.m_templ == b.m_templ && a.m_params < b.m_params);
}

/*******************************************************************************
  TemplateInstantiator
*******************************************************************************/

struct InstanceInfo {
    TreeNodeProcDef*  m_generatedBody;
    SymbolTable*      m_localScope;
    ModuleInfo*       m_moduleInfo;

    InstanceInfo ()
        : m_generatedBody (nullptr)
        , m_localScope (nullptr)
        , m_moduleInfo (nullptr)
    { }
};

class TemplateInstantiator {
private: /* Types: */

    typedef std::map<Instantiation, InstanceInfo> InstanceInfoMap;

public: /* Methods: */

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

    std::set<Instantiation>     m_generated; ///< set of generated instances
    InstanceInfoMap             m_instanceInfo;
    std::deque<Instantiation >  m_workList; ///<
};


} // namespace SecreC;

#endif
