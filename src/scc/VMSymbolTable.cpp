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

#include "VMSymbolTable.h"

#include <cassert>
#include <sharemind/Concat.h>
#include "VMValue.h"


namespace SecreCC {
namespace {

template <typename K, typename T>
T *
getOrCreate(std::unordered_map<K, std::unique_ptr<T> > & map, K const & key) {
    auto const it(map.find(key));
    if (it != map.end())
        return it->second.get();
    return map.emplace(key, std::make_unique<T>(key)).first->second.get();
}

} // namespace anonymous

VMSymbolTable::VMSymbolTable() = default;
VMSymbolTable::~VMSymbolTable() = default;

VMVReg * VMSymbolTable::getVReg(bool const isGlobal) {
    m_vregs.emplace_back(std::make_unique<VMVReg>(isGlobal));
    return m_vregs.back().get();
}

VMValue *
VMSymbolTable::find(SecreC::Symbol const * const symbol) const noexcept {
    assert(symbol);
    auto const it(m_mapping.find(symbol));
    return it != m_mapping.end() ? it->second : nullptr;
}

bool VMSymbolTable::store(SecreC::Symbol const * const symbol,
                          VMValue * const value)
{
    assert(symbol);
    assert(value);
    return m_mapping.emplace(symbol, value).second;
}

VMImm * VMSymbolTable::getImm(std::uint64_t const value)
{ return getOrCreate(m_imms, value); }

VMReg * VMSymbolTable::getReg (std::size_t const number)
{ return getOrCreate(m_globals, number); }

VMStack * VMSymbolTable::getStack(std::size_t const number)
{ return getOrCreate(m_locals, number); }

VMLabel * VMSymbolTable::getLabel(std::string const & name)
{ return getOrCreate(m_labels, name); }

VMLabel * VMSymbolTable::getUniqLabel()
{ return getLabel(sharemind::concat(":LU_", uniq())); }

} // namespace SecreCC {
