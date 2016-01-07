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

#ifndef SECREC_TYPES_H
#define SECREC_TYPES_H

#include "ParserEnums.h"
#include "PrettyPrint.h"

#include <cassert>
#include <vector>
#include <utility>
#include <iosfwd>
#include <tuple>

namespace SecreC {

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
    bool isString () const;
    bool isFloat () const;

    virtual const SecurityType* secrecSecType() const = 0;
    virtual SecrecDimType secrecDimType() const = 0;
    virtual const DataType* secrecDataType() const = 0;

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

    static const TypeVoid* get ();


    const SecurityType* secrecSecType() const override final {
        assert (false && "TypeVoid::secrecSecType");
        return nullptr;
    }

    const DataType* secrecDataType() const override final {
        assert (false && "TypeVoid::secrecDataType");
        return nullptr;
    }

    SecrecDimType secrecDimType() const override final {
        assert (false && "TypeVoid::secrecDimType");
        return (~ SecrecDimType(0));
    }

protected: /* Methods: */

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

    bool latticeLEQ (const TypeNonVoid* other) const;

protected: /* Methods: */

    TypeNonVoid (Kind kind)
        : Type (kind)
    { }
};

/*******************************************************************************
  TypeBasic
*******************************************************************************/

class TypeBasic : public TypeNonVoid {
public: /* Types: */

    struct Key;

public: /* Methods: */


    // For boost::flyweight
    explicit TypeBasic(const Key&);

    const SecurityType* secrecSecType() const override final { return m_secType; }
    SecrecDimType secrecDimType() const override final { return m_dimType; }
    const DataType* secrecDataType() const override final { return m_dataType; }

    static const TypeBasic* get (SecrecDataType dataType,
                                 SecrecDimType dimType = 0);
    static const TypeBasic* get (const DataType* dataType,
                                 SecrecDimType dimType = 0);
    static const TypeBasic* get (const SecurityType* secType,
                                 SecrecDataType dataType,
                                 SecrecDimType dimType = 0);
    static const TypeBasic* get (const SecurityType* secType,
                                 const DataType* dataType,
                                 SecrecDimType dimType = 0);
    static const TypeBasic* getIndexType ();
    static const TypeBasic* getPublicBoolType ();

protected: /* Methods: */

    explicit TypeBasic(const SecurityType* secType,
                       const DataType* dataType,
                       SecrecDimType dim)
        : TypeNonVoid (BASIC)
        , m_secType (secType)
        , m_dataType (dataType)
        , m_dimType (dim)
    { }

    void printPrettyV (std::ostream &os) const override;

private: /* Fields: */
    const SecurityType* const m_secType;
    const DataType* const m_dataType;
    SecrecDimType const m_dimType;
};

/*******************************************************************************
  TypeProc
*******************************************************************************/

class TypeProc : public TypeNonVoid {
public: /* Methods: */

    explicit TypeProc (const std::pair<
                            std::vector<const TypeBasic*>,
                            const Type*>& p)
        : TypeNonVoid (PROCEDURE)
        , m_params (p.first)
        , m_returnType (p.second)
    { }

    const Type* returnType () const { return m_returnType; }
    const std::vector<const TypeBasic*>& paramTypes() const { return m_params; }
    std::string mangle () const;
    std::string paramsToNormalString () const;

    const SecurityType* secrecSecType() const override final {
        return returnType ()->secrecSecType ();
    }

    const DataType* secrecDataType() const override final {
        return returnType ()->secrecDataType ();
    }

    SecrecDimType secrecDimType() const override final {
        return returnType ()->secrecDimType ();
    }

    static const TypeProc* get (const std::vector<const TypeBasic*>& params,
                                const Type* returnType = nullptr);

protected: /* Methods: */

    explicit TypeProc (std::vector<const TypeBasic*> params,
                       const Type* returnType)
        : TypeNonVoid (PROCEDURE)
        , m_params (std::move(params))
        , m_returnType (returnType)
    { }

    void printPrettyV (std::ostream &os) const override;

private: /* Fields: */
    std::vector<const TypeBasic*> const m_params;
    const Type* const m_returnType;
};


} // namespace SecreC


#endif // SECREC_H
