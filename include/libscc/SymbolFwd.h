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

#ifndef SECREC_SYMBOL_FWD_H
#define SECREC_SYMBOL_FWD_H

namespace SecreC {

enum SymbolCategory {
    SYM_UNDEFINED,
    SYM_PROCEDURE,
    SYM_STRUCT,
    SYM_PROCEDURE_TEMPLATE,
    SYM_OPERATOR_TEMPLATE,
    SYM_CONSTANT,
    SYM_LABEL,
    SYM_SYMBOL,
    SYM_TYPE,
    SYM_KIND,
    SYM_DOMAIN,
    SYM_DIM
};

class Symbol;
class SymbolProcedure;
class SymbolStruct;
class SymbolProcedureTemplate;
class SymbolOperatorTemplate;
class SymbolConstant;
class SymbolLabel;
class SymbolSymbol;
class SymbolKind;
class SymbolDomain;
class SymbolDimensionality;
class SymbolDataType;

template <SymbolCategory type> struct SymbolTraits;
template <> struct SymbolTraits<SYM_PROCEDURE> { using Type = SymbolProcedure; };
template <> struct SymbolTraits<SYM_STRUCT> { using Type = SymbolStruct; };
template <> struct SymbolTraits<SYM_PROCEDURE_TEMPLATE> { using Type = SymbolProcedureTemplate; };
template <> struct SymbolTraits<SYM_OPERATOR_TEMPLATE> { using Type = SymbolOperatorTemplate; };
template <> struct SymbolTraits<SYM_CONSTANT> { using Type = SymbolConstant; };
template <> struct SymbolTraits<SYM_LABEL> { using Type = SymbolLabel; };
template <> struct SymbolTraits<SYM_SYMBOL> { using Type = SymbolSymbol; };
template <> struct SymbolTraits<SYM_KIND> { using Type = SymbolKind; };
template <> struct SymbolTraits<SYM_DOMAIN> { using Type = SymbolDomain; };
template <> struct SymbolTraits<SYM_DIM> { using Type = SymbolDimensionality; };
template <> struct SymbolTraits<SYM_TYPE> { using Type = SymbolDataType; };
} // namespace SecreC

#endif // SECREC_SYMBOL_FWD_H
