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

enum SymbolType {
    SYM_PROCEDURE,
    SYM_TEMPLATE,
    SYM_CONSTANT,
    SYM_LABEL,
    SYM_SYMBOL,
    SYM_KIND,
    SYM_DOMAIN,
    SYM_DIM
};

class Symbol;
class SymbolProcedure;
class SymbolTemplate;
class SymbolConstant;
class SymbolLabel;
class SymbolSymbol;
class SymbolKind;
class SymbolDomain;
class SymbolDimensionality;

template <SymbolType type> struct SymbolTraits;
template <> struct SymbolTraits<SYM_PROCEDURE> { typedef SymbolProcedure Type; };
template <> struct SymbolTraits<SYM_TEMPLATE> { typedef SymbolTemplate Type; };
template <> struct SymbolTraits<SYM_CONSTANT> { typedef SymbolConstant Type; };
template <> struct SymbolTraits<SYM_LABEL> { typedef SymbolLabel Type; };
template <> struct SymbolTraits<SYM_SYMBOL> { typedef SymbolSymbol Type; };
template <> struct SymbolTraits<SYM_KIND> { typedef SymbolKind Type; };
template <> struct SymbolTraits<SYM_DOMAIN> { typedef SymbolDomain Type; };
template <> struct SymbolTraits<SYM_DIM> { typedef SymbolDimensionality Type; };

} // namespace SecreC

#endif // SECREC_SYMBOL_FWD_H
