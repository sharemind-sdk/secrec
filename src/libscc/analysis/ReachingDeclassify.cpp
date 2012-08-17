/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "analysis/ReachingDeclassify.h"

#include <boost/foreach.hpp>
#include <sstream>

#include "misc.h"
#include "symbol.h"
#include "treenode.h"

namespace SecreC {

namespace /* anonymous */ {

uint8_t maximumEntropy (SecrecDataType dtype) {
    switch (dtype) {
    case DATATYPE_BOOL:       return 1;
    case DATATYPE_INT8:       return 8;
    case DATATYPE_INT16:      return 16;
    case DATATYPE_INT32:      return 32;
    case DATATYPE_INT64:      return 64;
    case DATATYPE_UINT8:      return 8;
    case DATATYPE_UINT16:     return 16;
    case DATATYPE_UINT32:     return 32;
    case DATATYPE_UINT64:     return 64;
    case DATATYPE_XOR_UINT8:  return 8;
    case DATATYPE_XOR_UINT16: return 16;
    case DATATYPE_XOR_UINT32: return 32;
    case DATATYPE_XOR_UINT64: return 64;
    default:
        assert (false && "invalid private data type!");
        return 0x0;
    }
}

} // namespace anonymous

/*******************************************************************************
  ReachingDeclassify
*******************************************************************************/

void ReachingDeclassify::inFrom (const Block& from, Edge::Label label, const Block& to) {
    if (Edge::isGlobal (label))
        return;

    BOOST_FOREACH (const PDefs::value_type& pdef, m_outs[&from]) {
        PDefs& in = m_ins[&to];
        in[pdef.first].nonsensitive += pdef.second.nonsensitive;
        in[pdef.first].sensitive += pdef.second.sensitive;
        in[pdef.first].trivial &= pdef.second.trivial;
    }
}

void ReachingDeclassify::transferImop (const Imop& imop, PDefs& out) const {
    assert (imop.type () != Imop::DECLASSIFY);

    if (!imop.isExpr ()) {
        if (imop.type () != Imop::STORE)
            return;
    }


    /*
     * We track sensitivity in only two cases:
     */
    switch (imop.type ()) {
    case Imop::PARAM:
    case Imop::CALL:
        BOOST_FOREACH (const Symbol* dest, imop.defRange ()) {
            if (dest->secrecType ()->secrecSecType ()->isPublic ())
                continue;

            out[dest].setToSensitive (&imop);
        }

        return;
    default:
        break;
    }

    // Skip if private destination is not written to:
    const Symbol* dest = imop.dest ();
    assert (dest != 0);
    if (dest->secrecType ()->secrecSecType ()->isPublic ()) {
        return;
    }

    Defs& d = out[dest];

    switch (imop.type ()) {
    case Imop::STORE: {
            Defs& s = out[imop.arg1 ()];
            d.nonsensitive += s.nonsensitive;
            d.nonsensitive += d.sensitive;
            d.sensitive = s.sensitive;
            d.trivial &= s.trivial;
            break;
        }
    case Imop::ALLOC:
        if (imop.arg1 ()->secrecType ()->secrecSecType ()->isPublic ()) {
    case Imop::CLASSIFY:
            d.setToNonsensitive (&imop);
            d.trivial = true;
            break;
        }
        else {
    case Imop::LOAD:
    case Imop::COPY:
    case Imop::UMINUS:
    case Imop::UNEG:
    case Imop::ASSIGN:
            assert (imop.arg1 ()->secrecType ()->secrecSecType ()->isPrivate ());
            out[imop.dest ()] = out[imop.arg1 ()];
        }

        break;

    case Imop::CAST: {
            SecrecDataType destType = dest->secrecType ()->secrecDataType ();
            SecrecDataType srcType = imop.arg1 ()->secrecType ()->secrecDataType ();
            if (maximumEntropy (destType) >= maximumEntropy (srcType)) {
                d = out[imop.arg1 ()];
            }
            else {
                // some entropy is discarded
                d.setToNonsensitive (&imop);
                d.trivial = out[imop.arg1 ()].trivial;
            }
        }

        break;

    case Imop::ADD:
    case Imop::SUB: {
            Defs& arg1 = out[imop.arg1 ()];
            Defs& arg2 = out[imop.arg2 ()];
            if (arg1.trivial) { d = arg2; } else
            if (arg2.trivial) { d = arg1; } else {
                d.setToNonsensitive (&imop);
            }

            break;
        }
    default:
        d.setToNonsensitive (&imop);
        d.trivial = false;
        break;
    }
}

bool ReachingDeclassify::makeOuts (const Block& b, const PDefs& in, PDefs& out) {
    PDefs old = out;
    out = in;
    for (Block::const_iterator it = b.begin (), e = b.end (); it != e; ++ it) {
        if (it->type () == Imop::DECLASSIFY) {
            m_ds[&*it] = out[it->arg1 ()];
            continue;
        }

        transferImop (*it, out);
    }

    return old != out;
}

void ReachingDeclassify::finish () {
    m_outs.clear ();
    m_ins.clear ();

    DD oldDs (m_ds);
    m_ds.clear ();
    BOOST_FOREACH (DD::value_type& def, oldDs) {
        if (!def.second.trivial && !def.second.sensitive.empty ()) {
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
                if (dynamic_cast<TreeNodeVarInit*>(imop->creator ()) != 0) {
                    os << "parameter "
                       << static_cast<TreeNodeVarInit*>(imop->creator ())->variableName ()
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
