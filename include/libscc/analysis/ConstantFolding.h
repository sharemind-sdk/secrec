#ifndef SECREC_CONSTANT_FOLDING_H
#define SECREC_CONSTANT_FOLDING_H

#include "../DataflowAnalysis.h"

#include <boost/interprocess/containers/flat_map.hpp>

namespace SecreC {

class AbstractValue;
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

    inline bool operator == (Value const& other) const {
        return m_kind == other.m_kind && m_value == other.m_value;
    }

    bool isNac () const { return m_kind == NAC; }
    bool isUndef () const { return m_kind == UNDEF; }
    bool isConst () const { return m_kind == CONSTANT; }

    const AbstractValue* value () const { return m_value; }

    std::string toString () const;

    static Value nac () { return Value (NAC, nullptr); }
    static Value undef () { return Value (UNDEF, nullptr); }

    friend bool operator < (const Value& x, const Value& y);

private: /* Fields: */
    Kind                 m_kind;
    const AbstractValue* m_value;
};

inline bool operator < (const Value& x, const Value& y) {
    return std::tie (x.m_kind, x.m_value) < std::tie (y.m_kind, y.m_value);
}

/*******************************************************************************
  ConstantFolding
*******************************************************************************/

class ConstantFolding : public ForwardDataFlowAnalysis {
public:
    using SVM = boost::container::flat_map<const Symbol*, Value>; // symbol to value map
    using BSVM = std::map<const Block*, SVM>; // block to symbol to value map
public:

    ConstantFolding ();
    ConstantFolding (const ConstantFolding&) = delete;
    ConstantFolding& operator = (const ConstantFolding&) = delete;
    ~ConstantFolding ();

    virtual void start(const Program &bs);
    virtual void startBlock(const Block &b);
    virtual void inFrom(const Block &from, Edge::Label label, const Block &to);
    virtual bool finishBlock(const Block &b);
    virtual inline void finish();

    std::string toString (const Program &bs) const;

private:

    bool makeOuts (Block const& b, SVM const& in, SVM& out);
    void addConstant (const Symbol* sym);
    void transfer (SVM& val, const Imop& imop);
    Value getVal (SVM& val, const Symbol* sym);
    static void setVal (SVM& val, const Symbol* sym, Value x);

    SVM m_constants;
    BSVM m_ins;
    BSVM m_outs;
    ValueFactory* m_values;
};

} // namespace SecreC

#endif // CONSTANT_FOLDING_H
