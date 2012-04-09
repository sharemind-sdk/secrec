/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_REACHING_DECLASSIFY_H
#define SECREC_REACHING_DECLASSIFY_H

#include <map>
#include <set>

#include "../dataflowanalysis.h"

namespace SecreC {

/*******************************************************************************
  ReachingDeclassify
*******************************************************************************/

class ReachingDeclassify: public ForwardDataFlowAnalysis {
public: /* Types: */
    typedef std::set<const Imop*> ImopSet;
    struct Defs {
        ImopSet sensitive;
        ImopSet nonsensitive;
        inline bool operator== (const Defs& o) const {
            return (sensitive == o.sensitive)
                    && (nonsensitive == o.nonsensitive);
        }
    };

    typedef std::map<const Symbol*, Defs> PDefs;
    typedef std::map<const Block*, PDefs> RD;
    typedef std::map<const Imop*, Defs> DD;

public: /* Methods: */

    virtual void start (const Program& pr) {
        // Initialize the OUT set of the entry block:
        makeOuts (*pr.entryBlock (), m_ins[pr.entryBlock ()], m_outs[pr.entryBlock ()]);
    }

    virtual void startBlock (const Block& b) { m_ins[&b].clear (); }
    virtual void inFrom (const Block& from, const Block& to);
    virtual inline void inFromTrue (const Block& from, const Block& to) { inFrom (from, to); }
    virtual inline void inFromFalse (const Block& from, const Block& to) { inFrom (from, to); }
    virtual inline void inFromCallPass (const Block& from, const Block& to) { inFrom (from, to); }
    virtual inline bool finishBlock (const Block& b) { return makeOuts (b, m_ins[&b], m_outs[&b]); }
    virtual void finish ();

    std::string toString (const Program& bs) const;

private: /* Methods: */
    bool makeOuts (const Block& b, const PDefs& in, PDefs& out);

private: /* Fields: */
    RD m_ins;
    RD m_outs;
    DD m_ds;
};

} // namespace SecreC

#endif /* SECREC_REACHING_DECLASSIFY_H */
