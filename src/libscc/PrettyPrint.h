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

#ifndef SECREC_PRETTY_PRINT_H
#define SECREC_PRETTY_PRINT_H

#include <iosfwd>

namespace SecreC {


/**
 * \brief Interface for pretty printable things.
 */
class PrettyPrintable {
protected: /* Methods: */
    friend class PrettyPrint;
    virtual ~PrettyPrintable() noexcept;
    virtual void printPrettyV (std::ostream& os) const = 0;
};

class PrettyPrint {
public: /* Methods: */
    PrettyPrint (const PrettyPrintable& printable)
        : m_printable (printable)
    { }

    PrettyPrint (const PrettyPrintable* printable)
        : m_printable (*printable)
    { }

    void printPretty (std::ostream& os) const {
        m_printable.printPrettyV (os);
    }

private: /* Fields: */
    const PrettyPrintable& m_printable;
};

inline std::ostream& operator << (std::ostream& os, const PrettyPrint& p) {
    p.printPretty (os);
    return os;
}

} /* namespace SecreC */

#endif /* SECREC_PRETTY_PRINT_H */
