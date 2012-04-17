#include "StringLiterals.h"

#include <boost/foreach.hpp>

#include "VMSymbolTable.h"
#include "VMCode.h"

namespace {

/// \todo probably broken
std::string escape (const std::string& str) {
    std::ostringstream os;

    os << '\"';
    BOOST_FOREACH (char c, str) {
        switch (c) {
        case '\'': os << "\\\'"; break;
        case '\"': os << "\\\""; break;
        case '\?': os << "\\?";  break;
        case '\\': os << "\\\\"; break;
        case '\a': os << "\\a";  break;
        case '\b': os << "\\b";  break;
        case '\f': os << "\\f";  break;
        case '\n': os << "\\n";  break;
        case '\r': os << "\\r";  break;
        case '\t': os << "\\t";  break;
        case '\v': os << "\\v";  break;
        default:   os << c;      break;
        }
    }

    os << "\\0\"";
    return os.str ();
}

}

namespace SecreCC {

/*******************************************************************************
  StringLiterals
*******************************************************************************/

StringLiterals::StringLiterals ()
    : m_st (0)
    , m_dataSection (0)
{ }

StringLiterals::~StringLiterals () { }

void StringLiterals::init (VMSymbolTable& st, VMDataSection* section) {
    m_st = &st;
    m_dataSection = section;
}

StringLiterals::LiteralInfo StringLiterals::insert (const SecreC::ConstantString* str) {
    return insert (str->value ());
}

StringLiterals::LiteralInfo StringLiterals::insert (const std::string& str) {
    LitMap::iterator i = m_literals.find (str);
    if (i == m_literals.end ()) {
        std::stringstream os;
        os << ":STR_" << m_st->uniq ();
        VMLabel* label = m_st->getLabel (os.str ());
        const std::string& s = escape (str);
        m_dataSection->addRecord (label, "string", s);
        i = m_literals.insert (i, std::make_pair (str, LiteralInfo (label, str.size () + 1)));
    }

    return i->second;
}


} // namespace SecreCC
