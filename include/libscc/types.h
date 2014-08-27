#ifndef SECREC_TYPES_H
#define SECREC_TYPES_H

#include <cassert>
#include <vector>

#include "parser.h"
#include "SecurityType.h"
#include "DataType.h"

namespace SecreC {

class Context;

/*******************************************************************************
  Type
*******************************************************************************/

class Type {
private:
    Type (const Type&); // DO NOT IMPLEMENT
    void operator = (const Type&); // DO NOT IMPLEMENT
public: /* Types: */

    enum Kind {
        VOID,
        BASIC,
        PROCEDURE
    };

    class PrettyPrint {
    public: /* Methods: */
        explicit PrettyPrint (const Type* self) : m_self (*self) { }
        explicit PrettyPrint (const Type& self) : m_self (self) { }
        inline void operator () (std::ostream& os) const { m_self.prettyPrint (os); }
    private: /* Fields: */
        const Type& m_self;
    };

public: /* Methods: */

    virtual inline ~Type() { }

    inline Kind kind () const { return m_kind; }
    inline bool isVoid() const { return m_kind == VOID; }
    inline bool isScalar () const { return secrecDimType() == 0; }
    inline bool isPublicUIntScalar () const;

    virtual SecurityType* secrecSecType() const = 0;
    virtual SecrecDataType secrecDataType() const = 0;
    virtual SecrecDimType secrecDimType() const = 0;

protected: /* Methods: */

    explicit inline Type (Kind kind)
        : m_kind (kind)
    { }

    friend std::ostream& operator << (std::ostream& os, const Type& type);
    virtual void print (std::ostream& os) const = 0;
    virtual void prettyPrint (std::ostream& os) const = 0;

private: /* Fields: */
    const Kind m_kind;
};

/*******************************************************************************
  TypeVoid
*******************************************************************************/

class TypeVoid: public Type {
public: /* Methods: */

    static TypeVoid* get (Context& cxt);

    inline SecurityType* secrecSecType() const {
        assert (false && "TypeVoid::secrecSecType");
        return 0;
    }

    inline SecrecDataType secrecDataType() const {
        assert (false && "TypeVoid::secrecDataType");
        return DATATYPE_UNDEFINED;
    }

    inline SecrecDimType secrecDimType() const {
        assert (false && "TypeVoid::secrecDimType");
        return (~ SecrecDimType(0));
    }

protected: /* Methods: */

    friend class ContextImpl; // TODO: workaround

    inline TypeVoid ()
        : Type (VOID)
    { }

    void print (std::ostream& os) const;
    void prettyPrint (std::ostream& os) const;
};

/*******************************************************************************
  TypeNonVoid
*******************************************************************************/

class TypeNonVoid: public Type {
public: /* Methods: */

    // TODO: this is not pretty
    inline bool latticeLEQ (const TypeNonVoid* other) const {
        if (kind () != other->kind ())
            return false;

        SecrecDataType dataType = other->secrecDataType ();
        if (other->secrecSecType ()->isPrivate () && secrecSecType ()->isPublic ()) {
            dataType = dtypeDeclassify (dataType);
        }

        return     latticeSecTypeLEQ (secrecSecType (), other->secrecSecType ())
                && latticeDataTypeLEQ (secrecDataType (), dataType)
                && latticeDimTypeLEQ (secrecDimType (), other->secrecDimType ());
    }

protected: /* Methods: */

    TypeNonVoid (Kind kind)
        : Type (kind)
    { }
};

/*******************************************************************************
  TypeBasic
*******************************************************************************/

class TypeBasic : public TypeNonVoid {
public: /* Methods: */

    inline SecurityType* secrecSecType() const { return m_secType; }
    inline SecrecDimType secrecDimType() const { return m_dimType; }

    // TODO: change return type to DataType*
    inline SecrecDataType secrecDataType() const {
        return static_cast<DataTypePrimitive*>(m_dataType)->secrecDataType ();
    }

    static TypeBasic* get (Context& cxt, SecrecDataType dataType,
                           SecrecDimType dimType = 0);
    static TypeBasic* get (Context& cxt, SecurityType* secType,
                           SecrecDataType dataType,
                           SecrecDimType dimType = 0);
    static TypeBasic* get (Context& cxt, SecurityType* secType,
                           DataType* dataType,
                           SecrecDimType dimType = 0);
    static TypeBasic* getIndexType (Context& cxt);
    static TypeBasic* getPublicBoolType (Context& cxt);

protected: /* Methods: */

    TypeBasic(SecurityType* secType,
              DataType* dataType,
              SecrecDimType dim = 0)
        : TypeNonVoid (BASIC)
        , m_secType (secType)
        , m_dataType (dataType)
        , m_dimType (dim)
    { }

    void print (std::ostream& os) const;
    void prettyPrint (std::ostream& os) const;

private: /* Fields: */
    SecurityType*   const m_secType;
    DataType*       const m_dataType;
    SecrecDimType   const m_dimType;
};

/*******************************************************************************
  TypeProc
*******************************************************************************/

class TypeProc : public TypeNonVoid {
public: /* Methods: */

    Type* returnType () const { return m_returnType; }
    const std::vector<TypeBasic*>& paramTypes() const { return m_params; }
    std::string mangle () const;
    std::string paramsToNormalString () const;

    inline SecurityType* secrecSecType() const { return returnType ()->secrecSecType (); }
    inline SecrecDataType secrecDataType() const { return returnType ()->secrecDataType (); }
    inline SecrecDimType secrecDimType() const { return returnType ()->secrecDimType (); }

    static TypeProc* get (Context& cxt,
                          const std::vector<TypeBasic*>& params,
                          Type* returnType = 0);

protected: /* Methods: */

    explicit TypeProc (const std::vector<TypeBasic*>& params,
                       Type* returnType)
        : TypeNonVoid (PROCEDURE)
        , m_params (params)
        , m_returnType (returnType)
    { }

    void print (std::ostream& os) const;
    void prettyPrint (std::ostream& os) const;

private: /* Fields: */
    std::vector<TypeBasic*> const m_params;
    Type* const m_returnType;
};


inline bool Type::isPublicUIntScalar () const {
    return secrecDataType () == DATATYPE_UINT64 &&
           secrecSecType ()->isPublic () &&
           secrecDimType () == 0;
}

inline std::ostream &operator<<(std::ostream &out, const Type &type) {
    type.print (out);
    return out;
}

inline std::ostream& operator << (std::ostream& os, const Type::PrettyPrint& pp) {
    pp (os);
    return os;
}


} // namespace SecreC


#endif // SECREC_H
