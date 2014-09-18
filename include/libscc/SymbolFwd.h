/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_SYMBOL_FWD_H
#define SECREC_SYMBOL_FWD_H

namespace SecreC {

enum SymbolCategory {
    SYM_UNDEFINED,
    SYM_PROCEDURE,
    SYM_STRUCT,
    SYM_TEMPLATE,
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
class SymbolTemplate;
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
template <> struct SymbolTraits<SYM_TEMPLATE> { using Type = SymbolTemplate; };
template <> struct SymbolTraits<SYM_CONSTANT> { using Type = SymbolConstant; };
template <> struct SymbolTraits<SYM_LABEL> { using Type = SymbolLabel; };
template <> struct SymbolTraits<SYM_SYMBOL> { using Type = SymbolSymbol; };
template <> struct SymbolTraits<SYM_KIND> { using Type = SymbolKind; };
template <> struct SymbolTraits<SYM_DOMAIN> { using Type = SymbolDomain; };
template <> struct SymbolTraits<SYM_DIM> { using Type = SymbolDimensionality; };
template <> struct SymbolTraits<SYM_TYPE> { using Type = SymbolDataType; };
} // namespace SecreC

#endif // SECREC_SYMBOL_FWD_H
