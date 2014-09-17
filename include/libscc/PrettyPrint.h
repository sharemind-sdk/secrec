/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
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
