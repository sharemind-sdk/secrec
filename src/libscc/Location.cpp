/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Location.h"

#include <ostream>

namespace SecreC {

Location::FilenameCache Location::m_filenameCache;


std::ostream & operator<<(std::ostream & os, const Location & loc) {
    os << loc.filename()
       << ":(" << loc.firstLine()
       << ',' << loc.firstColumn()
       << ")(" << loc.lastLine()
       << ',' << loc.lastColumn() << ')';
    return os;
}

}
