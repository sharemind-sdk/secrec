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

#include "StringRef.h"

#include "StringTable.h"

#include <ostream>

namespace SecreC {

std::ostream& operator << (std::ostream& os, StringRef sref) {
    os.write (sref.data (), sref.size ());
    return os;
}


} // namespace SecreC

extern "C" {

const SecreC::StringRef* add_string (SecreC::StringTable* table, const char * str, size_t size) {
    return table->addString(str, size);
}

size_t stringref_length (const SecreC::StringRef* ref) {
    return ref->size();
}

const char* stringref_begin (const SecreC::StringRef* ref) {
    return ref->data();
}

}

