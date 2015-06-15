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

#ifndef SECREC_CONSTANT_FOLDING_H
#define SECREC_CONSTANT_FOLDING_H

#include "../DataflowAnalysis.h"

#include <boost/interprocess/containers/flat_map.hpp>

namespace SecreC {

class AbstractValue;
class Context;
class StringTable;
class ValueFactory;

/*******************************************************************************
  Value
*******************************************************************************/

class Value {
public: /* Types: */

    enum Kind {
        UNDEF,
        CONSTANT,
        NAC
    };

public: /* Methods: */

    Value ()
        : m_kind (UNDEF)
        , m_value (nullptr)
    { }

    Value (Kind kind, const AbstractValue* value)
        : m_kind (kind)
        , m_value (value)
    { }

    Value (const AbstractValue* value)
        : m_kind (CONSTANT)
        , m_value (value)
    { }

    bool isNac () const { return m_kind == NAC; }
    bool isUndef () const { return m_kind == UNDEF; }
    bool isConst () const { return m_kind == CONSTANT; }

    const AbstractValue* value () const { return m_value; }

    std::string toString () const;

    static Value nac () { return Value (NAC, nullptr); }
    static Value undef () { return Value (UNDEF, nullptr); }

    friend bool operator < (Value x, Value y);
    friend bool operator == (Value x, Value y);
    friend bool operator != (Value x, Value y);

private: /* Fields: */
    Kind                 m_kind;
    const AbstractValue* m_value;
};

// This is lexicographic not lattice ordering!
inline bool operator < (Value x, Value y) {
    return std::tie (x.m_kind, x.m_value) < std::tie (y.m_kind, y.m_value);
}

inline bool operator == (Value x, Value y) {
    return x.m_kind == y.m_kind && x.m_value == y.m_value;
}

inline bool operator != (Value x, Value y) {
    return !(x == y);
}

/*******************************************************************************
  ConstantFolding
*******************************************************************************/

class ConstantFolding final: public ForwardDataFlowAnalysis {
public: /* Types: */
    using SVM = boost::container::flat_map<const Symbol*, Value>; // symbol to value map
    using BSVM = std::map<const Block*, SVM>; // block to symbol to value map

public: /* Methods: */

    ConstantFolding ();
    ConstantFolding (const ConstantFolding&) = delete;
    ConstantFolding& operator = (const ConstantFolding&) = delete;
    ~ConstantFolding ();

    void start(const Program &bs) override final;
    void startBlock(const Block &b) override final;
    void inFrom(const Block &from, Edge::Label label, const Block &to) override final;
    bool finishBlock(const Block &b) override final;
    void finish() final override;

    size_t optimizeBlock(Context& cxt, StringTable& st, Block& block) const;

    std::string toString (const Program &bs) const override final;

private:

    bool makeOuts (Block const& b, SVM const& in, SVM& out);
    void addConstant (const Symbol* sym);
    void transfer (SVM& val, const Imop& imop) const;
    Value getVal (const SVM& val, const Symbol* sym) const;
    static void setVal (SVM& val, const Symbol* sym, Value x);

private: /* Fields: */
    SVM m_constants;
    BSVM m_ins;
    BSVM m_outs;
    ValueFactory* m_values;
};

} // namespace SecreC

#endif // CONSTANT_FOLDING_H
