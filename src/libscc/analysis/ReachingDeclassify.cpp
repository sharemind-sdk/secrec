/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "analysis/ReachingDeclassify.h"

#include "symbol.h"
#include "treenode.h"

#include <sstream>

#include <boost/foreach.hpp>

namespace /* anonymous */ {

template <class T, class U>
inline std::set<T>& operator+= (std::set<T>& dest, const std::set<U>& src) {
    dest.insert (src.begin (), src.end ());
    return dest;
}

} // namespace anonymous

namespace SecreC {

/*******************************************************************************
  ReachingDeclassify
*******************************************************************************/

void ReachingDeclassify::inFrom (const Block& from, const Block& to) {
    BOOST_FOREACH (const PDefs::value_type& pdef, m_outs[&from]) {
        m_ins[&to][pdef.first].nonsensitive += pdef.second.nonsensitive;
        m_ins[&to][pdef.first].sensitive += pdef.second.sensitive;
    }
}

bool ReachingDeclassify::makeOuts (const Block& b, const PDefs& in, PDefs& out) {
    PDefs old = out;
    out = in;
    for (Block::const_iterator it = b.begin (), e = b.end (); it != e; ++ it) {
        const Imop& imop = *it;
        if (!imop.isExpr ()) {
            if (imop.type () != Imop::PARAM)
                continue;
        } else {
            if (imop.dest () == 0) continue;
            if (imop.type () == Imop::DECLASSIFY) {
                m_ds[&imop] = out[imop.arg1 ()];
                continue;
            }
        }

        if (imop.dest ()->secrecType ()->secrecSecType ()->isPublic ())
            continue;

        Defs& d = out[imop.dest ()];

        switch (imop.type ()) {
        case Imop::PARAM:
        case Imop::CALL:
            d.nonsensitive.clear ();
            d.sensitive.clear ();
            d.sensitive.insert (&imop);
            break;
        case Imop::ASSIGN:
        case Imop::UMINUS:
        case Imop::UNEG:
            if (imop.arg1 ()->symbolType () != Symbol::CONSTANT) {
                if (imop.dest () != imop.arg1 ()) {
                    d = out[imop.arg1 ()];
                }
            } else {
                d.nonsensitive.clear ();
                d.nonsensitive.insert (&imop);
                d.sensitive.clear ();
            }

            break;
        default:
            d.nonsensitive.clear ();
            d.nonsensitive.insert (&imop);
            d.sensitive.clear ();
            break;
        }
    }

    return old != out;
}

void ReachingDeclassify::finish () {
    m_outs.clear ();
    m_ins.clear ();

    DD oldDs (m_ds);
    m_ds.clear ();
    BOOST_FOREACH (DD::value_type& def, oldDs) {
        if (!def.second.sensitive.empty ()) {
            m_ds.insert (def);
        }
    }
}

std::string ReachingDeclassify::toString (const Program&) const {
    assert(m_ins.empty ());
    std::ostringstream os;
    os << "Trivial declassify analysis results:" << std::endl;
    if (m_ds.empty ()) {
        os << "    No trivial leaks found! :)" << std::endl;
        return os.str ();
    }

    BOOST_FOREACH (const DD::value_type& defs, m_ds) {
        os << "    declassify at "
           << defs.first->creator ()->location ()
           << (defs.second.nonsensitive.empty () ? " leaks the value from:" : " might fully leak the value from:")
           << std::endl;
        BOOST_FOREACH (const Imop* imop, defs.second.sensitive) {
            os << "        ";
            switch (imop->type ()) {
            case Imop::PARAM:
            case Imop::CALL:
                if (dynamic_cast<TreeNodeStmtDecl*>(imop->creator ()) != 0) {
                    os << "parameter "
                       << static_cast<TreeNodeStmtDecl*>(imop->creator ())->variableName ()
                       << " declared at " << imop->creator ()->location ();
                } else {
                    assert (dynamic_cast<TreeNodeExprProcCall*>(imop->creator ()) != 0);
                    TreeNodeExprProcCall *c = static_cast<TreeNodeExprProcCall*>(imop->creator ());
                    assert (c->symbolProcedure () != 0);
                    assert (c->symbolProcedure ()->decl () != 0);
                    os << "call to " << c->symbolProcedure ()->decl ()->procedureName ()
                       << " at " << c->location ();
                }
                break;
            default:
                assert (false);
            }
            os << std::endl;
        }
    }

    return os.str ();
}

} // namespace SecreC
