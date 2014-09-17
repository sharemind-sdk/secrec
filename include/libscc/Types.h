/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_TYPES_H
#define SECREC_TYPES_H

#include "ParserEnums.h"
#include "PrettyPrint.h"

#include <cassert>
#include <vector>
#include <utility>
#include <iosfwd>

namespace SecreC {

class Context;
class SecurityType;
class DataType;

/*******************************************************************************
  Type
*******************************************************************************/

class Type: public PrettyPrintable {
public: /* Types: */

    enum Kind {
        VOID,
        BASIC,
        PROCEDURE
    };

public: /* Methods: */

    Type (const Type&) = delete;
    Type& operator = (const Type&) = delete;
    virtual ~Type() { }

    Kind kind () const { return m_kind; }
    bool isVoid() const { return m_kind == VOID; }
    bool isScalar () const { return secrecDimType() == 0; }
    bool isPublicUIntScalar () const;

    virtual SecurityType* secrecSecType() const = 0;
    virtual SecrecDimType secrecDimType() const = 0;
    virtual DataType* secrecDataType() const = 0;

protected: /* Methods: */

    explicit inline Type (Kind kind)
        : m_kind (kind)
    { }

private: /* Fields: */
    const Kind m_kind;
};

/*******************************************************************************
  TypeVoid
*******************************************************************************/

class TypeVoid: public Type {
public: /* Methods: */

    static TypeVoid* get (Context& cxt);


    SecurityType* secrecSecType() const override final {
        assert (false && "TypeVoid::secrecSecType");
        return nullptr;
    }

    DataType* secrecDataType() const override final {
        assert (false && "TypeVoid::secrecDataType");
        return nullptr;
    }

    SecrecDimType secrecDimType() const override final {
        assert (false && "TypeVoid::secrecDimType");
        return (~ SecrecDimType(0));
    }

protected: /* Methods: */

    friend class ContextImpl;

    inline TypeVoid ()
        : Type (VOID)
    { }

    void printPrettyV (std::ostream &os) const override;
};

/*******************************************************************************
  TypeNonVoid
*******************************************************************************/

class TypeNonVoid: public Type {
public: /* Methods: */

    // TODO: this is not pretty
    bool latticeLEQ (Context& cxt, const TypeNonVoid* other) const;

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

    SecurityType* secrecSecType() const override final { return m_secType; }
    SecrecDimType secrecDimType() const override final { return m_dimType; }
    DataType* secrecDataType() const override final { return m_dataType; }

    static TypeBasic* get (Context& cxt, SecrecDataType dataType,
                           SecrecDimType dimType = 0);
    static TypeBasic* get (Context& cxt, DataType* dataType,
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

    void printPrettyV (std::ostream &os) const override;

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

    SecurityType* secrecSecType() const override final {
        return returnType ()->secrecSecType ();
    }

    DataType* secrecDataType() const override final {
        return returnType ()->secrecDataType ();
    }

    SecrecDimType secrecDimType() const override final {
        return returnType ()->secrecDimType ();
    }

    static TypeProc* get (Context& cxt,
                          const std::vector<TypeBasic*>& params,
                          Type* returnType = nullptr);

protected: /* Methods: */

    explicit TypeProc (std::vector<TypeBasic*> params,
                       Type* returnType)
        : TypeNonVoid (PROCEDURE)
        , m_params (std::move(params))
        , m_returnType (returnType)
    { }

    void printPrettyV (std::ostream &os) const override;

private: /* Fields: */
    std::vector<TypeBasic*> const m_params;
    Type* const m_returnType;
};


} // namespace SecreC


#endif // SECREC_H
