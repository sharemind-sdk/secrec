/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

/**
* \cond
*/
module a3p_oblivious;

import stdlib;
/**
* \endcond
*/

/**
* @file
* \defgroup a3p_oblivious a3p_oblivious.sc
* \defgroup a3p_choose choose
*/

/** \addtogroup <a3p_oblivious> 
*@{
* @brief Module with functions for oblivious tasks (additive3pp protection domain)
*/

/** \addtogroup <a3p_choose> 
 *  @{
 *  @brief Function for obliviously choosing one of the inputs
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @param cond - a boolean array.
 *  @return returns one of the input arrays that was obliviously chosen with the condition. if **true**, array **first** is returned else **second** is returned 
 */

template <domain D : additive3pp, dim N>
D float32[[N]] choose(D bool[[N]] cond, D float32[[N]] first, D float32[[N]] second) {
    D float32[[N]] out = first;
    __syscall ("additive3pp::choose_float32_vec", __domainid (D), cond, first, second, out);
    return out;
}

template <domain D : additive3pp, dim N>
D float64[[N]] choose(D bool[[N]] cond, D float64[[N]] first, D float64[[N]] second) {
    D float64[[N]] out = first;
    __syscall ("additive3pp::choose_float64_vec", __domainid (D), cond, first, second, out);
    return out;
}

/** @}*/
/** @}*/

