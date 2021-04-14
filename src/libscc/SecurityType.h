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

#ifndef SECREC_SECURITYTYPE_H
#define SECREC_SECURITYTYPE_H

#include <iosfwd>
#include <sharemind/StringView.h>
#include <utility>


namespace SecreC {

class SymbolKind;

/*******************************************************************************
  SecurityType
*******************************************************************************/

class SecurityType {
private:
    SecurityType (const SecurityType&) = delete;
    SecurityType& operator = (const SecurityType&) = delete;

public: /* Methods: */
    virtual ~SecurityType () { }
    inline bool isPrivate () const { return !m_isPublic; }
    inline bool isPublic () const { return m_isPublic; }

protected:
    virtual void print (std::ostream& os) const = 0;
    friend std::ostream& operator<<(std::ostream &out, const SecurityType& type);

    explicit SecurityType (bool isPublic)
        : m_isPublic (isPublic)
    { }

private: /* Fields: */
    const bool m_isPublic;
};

inline std::ostream &operator<<(std::ostream &out, const SecurityType& type) {
    type.print (out);
    return out;
}

/*******************************************************************************
  PublicSecType
*******************************************************************************/

class PublicSecType : public SecurityType {
public: /* Methods: */
    PublicSecType ()
        : SecurityType (true)
    { }

    static const PublicSecType* get ();

protected:
    void print (std::ostream & os) const override;
};

/*******************************************************************************
  PrivateSecType
*******************************************************************************/

class PrivateSecType : public SecurityType {
public: /* Methods: */

    // For boost::flyweight
    PrivateSecType(std::pair<sharemind::StringView, SymbolKind *> const & p)
        : SecurityType (false)
        , m_name (p.first)
        , m_kind (p.second)
    { }

    PrivateSecType(sharemind::StringView name, SymbolKind * kind)
        : SecurityType (false)
        , m_name (std::move(name))
        , m_kind (kind)
    { }

    sharemind::StringView name() const noexcept { return m_name; }
    inline SymbolKind* securityKind () const { return m_kind; }

    static PrivateSecType const * get(sharemind::StringView name,
                                      SymbolKind * kind);

protected:
    void print (std::ostream & os) const override;

private: /* Fields: */

    sharemind::StringView const m_name;

    SymbolKind* const m_kind;
};

inline bool latticeSecTypeLEQ (const SecurityType* a, const SecurityType* b) {
    if (a->isPublic ()) return true;
    if (b->isPublic ()) return false;
    return a == b;
}

/// \retval 0 if the upper security type is not defined
inline const SecurityType* upperSecType (const SecurityType* a, const SecurityType* b) {
    if (a->isPublic ()) return b;
    if (b->isPublic ()) return a;
    if (a == b) return a;
    return nullptr;
}

} // namespace SecreC

#endif // SECREC_SECURITYTYPE_H
