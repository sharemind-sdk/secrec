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

#ifndef SECREC_REACHING_DECLASSIFY_H
#define SECREC_REACHING_DECLASSIFY_H

#include "../DataflowAnalysis.h"

namespace SecreC {

/*******************************************************************************
  ReachingDeclassify
*******************************************************************************/

class ReachingDeclassify: public ForwardDataFlowAnalysis {
public: /* Types: */

    using ImopSet = std::set<const Imop*>;

    struct Defs {
        ImopSet  sensitive;
        ImopSet  nonsensitive;
        bool     trivial;

        Defs () : trivial (true) { }

        void setToSensitive (const Imop* imop) {
            nonsensitive.clear ();
            sensitive.clear ();
            sensitive.insert (imop);
            trivial = false;
        }

        void setToNonsensitive (const Imop* imop) {
            sensitive.clear ();
            nonsensitive.clear ();
            nonsensitive.insert (imop);
        }

        inline bool operator== (const Defs& o) const {
            return (sensitive == o.sensitive)
                    && (nonsensitive == o.nonsensitive)
                    && (trivial == o.trivial);
        }
    };

    using PDefs = std::map<const Symbol*, Defs>;
    using RD = std::map<const Block*, PDefs>;
    using DD = std::map<const Imop*, Defs>;

public: /* Methods: */

    std::string toString (const Program& bs) const override;

protected:

    virtual void start (const Program& pr) override {
        // Initialize the OUT set of the entry block:
        makeOuts (*pr.entryBlock (), m_ins[pr.entryBlock ()], m_outs[pr.entryBlock ()]);
    }

    virtual void startBlock (const Block& b) override { m_ins[&b].clear (); }
    virtual void inFrom (const Block& from, Edge::Label label, const Block& to) override;
    virtual bool finishBlock (const Block& b) override { return makeOuts (b, m_ins[&b], m_outs[&b]); }
    virtual void finish () override;

private:

    bool makeOuts (const Block& b, const PDefs& in, PDefs& out);
    void transferImop (const Imop& imop, PDefs& out) const;

private: /* Fields: */
    RD m_ins;
    RD m_outs;
    DD m_ds;
};

} // namespace SecreC

#endif /* SECREC_REACHING_DECLASSIFY_H */
