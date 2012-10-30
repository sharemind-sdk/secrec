/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
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

