/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

/**
 * @file
 * \defgroup xor3pp xor3pp.sc
 * \defgroup xor3pp_bit_extract bit extraction
 * \defgroup xor3pp_reshare reshare
 * \defgroup xor3pp_choose choose
 * \defgroup xor3pp_min min
 * \defgroup xor3pp_min_vec min
 * \defgroup xor3pp_min_2 min(2 vectors)
 * \defgroup xor3pp_max max
 * \defgroup xor3pp_max_vec max
 * \defgroup xor3pp_max_2 max(2 vectors)
 */

/**
 * \cond
 */
module xor3pp;
/**
 * \endcond
 */

/**
 * \addtogroup <xor3pp>
 * @{
 * @brief Module with functions for xor_uint(X) data types
 */

/**
 * \addtogroup <xor3pp_bit_extract>
 * @{
 * @brief Function for converting xor_uint(X) type value to the bit representation.
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @note The input is arbitrary dimensional array, output is flattened to one boolean vector. Reshape the result to get appropriate dimensionality.
 *  @param input - the input value to convert
 *  @return returns filattened vector of extracted bits
 */

template <domain D : additive3pp, dim N>
D bool[[1]] bit_extract (D xor_uint8[[N]] input) {
    D bool[[1]] out (8 * size (input));
    __syscall ("additive3pp::bit_extract_xor_uint8_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp, dim N>
D bool[[1]] bit_extract (D xor_uint16[[N]] input) {
    D bool[[1]] out (16 * size (input));
    __syscall ("additive3pp::bit_extract_xor_uint16_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp, dim N>
D bool[[1]] bit_extract (D xor_uint32[[N]] input) {
    D bool[[1]] out (32 * size (input));
    __syscall ("additive3pp::bit_extract_xor_uint32_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp, dim N>
D bool[[1]] bit_extract (D xor_uint64[[N]] input) {
    D bool[[1]] out (64 * size (input));
    __syscall ("additive3pp::bit_extract_xor_uint64_vec", __domainid (D), input, out);
    return out;
}

/**
 * @}
 * \addtogroup <xor3pp_reshare>
 * @{
 * @brief Function for converting uint(X) type values to xor_uint(X) and the other way around
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @param input - the input value to convert
 *  @return returns a converted value from uint(X) -> xor_uint(X) or xor_uint(X) -> uint(X)
 */

template <domain D : additive3pp>
D xor_uint8 reshare (D uint8 input) {
    D xor_uint8 out;
    __syscall ("additive3pp::reshare_uint8_to_xor_uint8_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint8 reshare (D xor_uint8 input) {
    D uint8 out;
    __syscall ("additive3pp::reshare_xor_uint8_to_uint8_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D xor_uint8 [[1]] reshare (D uint8[[1]] input) {
    D xor_uint8[[1]] out (size (input));
    __syscall ("additive3pp::reshare_uint8_to_xor_uint8_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint8 [[1]] reshare (D xor_uint8[[1]] input) {
    D uint8[[1]] out (size (input));
    __syscall ("additive3pp::reshare_xor_uint8_to_uint8_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D xor_uint8 [[2]] reshare (D uint8[[2]] input) {
    D xor_uint8[[2]] out (shape(input)[0],shape(input)[1]);
    __syscall ("additive3pp::reshare_uint8_to_xor_uint8_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint8 [[2]] reshare (D xor_uint8[[2]] input) {
    D uint8[[2]] out (shape(input)[0],shape(input)[1]);
    __syscall ("additive3pp::reshare_xor_uint8_to_uint8_vec", __domainid (D), input, out);
    return out;
}

/*****************************
*****************************/

template <domain D : additive3pp>
D xor_uint16 reshare (D uint16 input) {
    D xor_uint16 out;
    __syscall ("additive3pp::reshare_uint16_to_xor_uint16_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint16 reshare (D xor_uint16 input) {
    D uint16 out;
    __syscall ("additive3pp::reshare_xor_uint16_to_uint16_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D xor_uint16 [[1]] reshare (D uint16[[1]] input) {
    D xor_uint16[[1]] out (size (input));
    __syscall ("additive3pp::reshare_uint16_to_xor_uint16_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint16 [[1]] reshare (D xor_uint16[[1]] input) {
    D uint16[[1]] out (size (input));
    __syscall ("additive3pp::reshare_xor_uint16_to_uint16_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D xor_uint16 [[2]] reshare (D uint16[[2]] input) {
    D xor_uint16[[2]] out (shape(input)[0],shape(input)[1]);
    __syscall ("additive3pp::reshare_uint16_to_xor_uint16_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint16 [[2]] reshare (D xor_uint16[[2]] input) {
    D uint16[[2]] out (shape(input)[0],shape(input)[1]);
    __syscall ("additive3pp::reshare_xor_uint16_to_uint16_vec", __domainid (D), input, out);
    return out;
}

/*****************************
*****************************/

template <domain D : additive3pp>
D xor_uint32 reshare (D uint32 input) {
    D xor_uint32 out;
    __syscall ("additive3pp::reshare_uint32_to_xor_uint32_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint32 reshare (D xor_uint32 input) {
    D uint32 out;
    __syscall ("additive3pp::reshare_xor_uint32_to_uint32_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D xor_uint32 [[1]] reshare (D uint32[[1]] input) {
    D xor_uint32[[1]] out (size (input));
    __syscall ("additive3pp::reshare_uint32_to_xor_uint32_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint32 [[1]] reshare (D xor_uint32[[1]] input) {
    D uint32[[1]] out (size (input));
    __syscall ("additive3pp::reshare_xor_uint32_to_uint32_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D xor_uint32 [[2]] reshare (D uint32[[2]] input) {
    D xor_uint32[[2]] out (shape(input)[0],shape(input)[1]);
    __syscall ("additive3pp::reshare_uint32_to_xor_uint32_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint32 [[2]] reshare (D xor_uint32[[2]] input) {
    D uint32[[2]] out (shape(input)[0],shape(input)[1]);
    __syscall ("additive3pp::reshare_xor_uint32_to_uint32_vec", __domainid (D), input, out);
    return out;
}

/*****************************
*****************************/

template <domain D : additive3pp>
D xor_uint64 reshare (D uint64 input) {
    D xor_uint64 out;
    __syscall ("additive3pp::reshare_uint64_to_xor_uint64_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint64 reshare (D xor_uint64 input) {
    D uint64 out;
    __syscall ("additive3pp::reshare_xor_uint64_to_uint64_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D xor_uint64 [[1]] reshare (D uint64[[1]] input) {
    D xor_uint64[[1]] out (size (input));
    __syscall ("additive3pp::reshare_uint64_to_xor_uint64_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint64 [[1]] reshare (D xor_uint64[[1]] input) {
    D uint64[[1]] out (size (input));
    __syscall ("additive3pp::reshare_xor_uint64_to_uint64_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D xor_uint64 [[2]] reshare (D uint64[[2]] input) {
    D xor_uint64[[2]] out (shape(input)[0],shape(input)[1]);
    __syscall ("additive3pp::reshare_uint64_to_xor_uint64_vec", __domainid (D), input, out);
    return out;
}

template <domain D : additive3pp>
D uint64 [[2]] reshare (D xor_uint64[[2]] input) {
    D uint64[[2]] out (shape(input)[0],shape(input)[1]);
    __syscall ("additive3pp::reshare_xor_uint64_to_uint64_vec", __domainid (D), input, out);
    return out;
}


/**
 * @}
 * \addtogroup <xor3pp_choose>
 *  @{
 *  @brief Function for obliviously choosing pointwise from the inputs
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @param cond - a boolean vector
 *  @return pointwise check if **cond** at a certain position is **true** or **false**. if **true** the element of **first** at that position is returned else the element of **second** at that position is returned
 */

template <domain D : additive3pp, dim N>
D xor_uint8[[N]] choose(D bool[[N]] cond, D xor_uint8[[N]] first, D xor_uint8[[N]] second) {
    D xor_uint8[[N]] out = first;
    __syscall ("additive3pp::choose_xor_uint8_vec", __domainid (D), cond, first, second, out);
    return out;
}

template <domain D : additive3pp, dim N>
D xor_uint16[[N]] choose(D bool[[N]] cond, D xor_uint16[[N]] first, D xor_uint16[[N]] second) {
    D xor_uint16[[N]] out = first;
    __syscall ("additive3pp::choose_xor_uint16_vec", __domainid (D), cond, first, second, out);
    return out;
}

template <domain D : additive3pp, dim N>
D xor_uint32[[N]] choose(D bool[[N]] cond, D xor_uint32[[N]] first, D xor_uint32[[N]] second) {
    D xor_uint32[[N]] out = first;
    __syscall ("additive3pp::choose_xor_uint32_vec", __domainid (D), cond, first, second, out);
    return out;
}

template <domain D : additive3pp, dim N>
D xor_uint64[[N]] choose(D bool[[N]] cond, D xor_uint64[[N]] first, D xor_uint64[[N]] second) {
    D xor_uint64[[N]] out = first;
    __syscall ("additive3pp::choose_xor_uint64_vec", __domainid (D), cond, first, second, out);
    return out;
}

/** @}*/
/*******************************
    Min, max
********************************/

/** \addtogroup <xor3pp_min>
 *  @{
 *  @brief Functions for finding the minimum value
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 */

/** \addtogroup <xor3pp_min_vec>
 *  @{
 *  @brief Function for finding the minimum element of the input vector.
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @returns minimum element of the input vector.
 *  @pre input vector is not empty
 */

template <domain D : additive3pp>
D xor_uint8 min (D xor_uint8[[1]] x) {
    assert (size(x) > 0);
    D xor_uint8 out;
    __syscall ("additive3pp::vecmin_xor_uint8_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D xor_uint16 min (D xor_uint16[[1]] x) {
    assert (size(x) > 0);
    D xor_uint16 out;
    __syscall ("additive3pp::vecmin_xor_uint16_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D xor_uint32 min (D xor_uint32[[1]] x) {
    assert (size(x) > 0);
    D xor_uint32 out;
    __syscall ("additive3pp::vecmin_xor_uint32_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D xor_uint64 min (D xor_uint64[[1]] x) {
    assert (size(x) > 0);
    D xor_uint64 out;
    __syscall ("additive3pp::vecmin_xor_uint64_vec", __domainid (D), x, out);
    return out;
}

/** @}*/
/** \addtogroup <xor3pp_min_2>
 *  @{
 *  @brief Function for finding the pointwise minimum of 2 arrays
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @returns an array with the pointwise minimum of each element in the two input vectors
 *  @pre both input vectors are of equal length
 */

template <domain D : additive3pp>
D xor_uint8 min (D xor_uint8 x, D xor_uint8 y) {
    __syscall ("additive3pp::min_xor_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint16 min (D xor_uint16 x, D xor_uint16 y) {
    __syscall ("additive3pp::min_xor_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint32 min (D xor_uint32 x, D xor_uint32 y) {
    __syscall ("additive3pp::min_xor_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint64 min (D xor_uint64 x, D xor_uint64 y) {
    __syscall ("additive3pp::min_xor_uint64_vec", __domainid (D), x, y, x);
    return x;
}

template <domain D : additive3pp>
D xor_uint8[[1]] min (D xor_uint8[[1]] x, D xor_uint8[[1]] y) {
    assert (size(x) == size(y));
    __syscall ("additive3pp::min_xor_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint16[[1]] min (D xor_uint16[[1]] x, D xor_uint16[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::min_xor_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint32[[1]] min (D xor_uint32[[1]] x, D xor_uint32[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::min_xor_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint64[[1]] min (D xor_uint64[[1]] x, D xor_uint64[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::min_xor_uint64_vec", __domainid (D), x, y, x);
    return x;
}

template <domain D : additive3pp, dim N>
D xor_uint8[[N]] min (D xor_uint8[[N]] x, D xor_uint8[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::min_xor_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D xor_uint16[[N]] min (D xor_uint16[[N]] x, D xor_uint16[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::min_xor_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D xor_uint32[[N]] min (D xor_uint32[[N]] x, D xor_uint32[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::min_xor_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D xor_uint64[[N]] min (D xor_uint64[[N]] x, D xor_uint64[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::min_xor_uint64_vec", __domainid (D), x, y, x);
    return x;
}

/** @}*/
/** @}*/


/** \addtogroup <xor3pp_max>
 *  @{
 *  @brief Functions for finding the maximum value
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 */

/** \addtogroup <xor3pp_max_vec>
 *  @{
 *  @brief Function for finding the maximum element of the input vector.
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @returns maximum element of the input vector.
 *  @pre input vector is not empty
 */

template <domain D : additive3pp>
D xor_uint8 max (D xor_uint8[[1]] x) {
    assert (size(x) > 0);
    D xor_uint8 out;
    __syscall ("additive3pp::vecmax_xor_uint8_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D xor_uint16 max (D xor_uint16[[1]] x) {
    assert (size(x) > 0);
    D xor_uint16 out;
    __syscall ("additive3pp::vecmax_xor_uint16_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D xor_uint32 max (D xor_uint32[[1]] x) {
    assert (size(x) > 0);
    D xor_uint32 out;
    __syscall ("additive3pp::vecmax_xor_uint32_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D xor_uint64 max (D xor_uint64[[1]] x) {
    assert (size(x) > 0);
    D xor_uint64 out;
    __syscall ("additive3pp::vecmax_xor_uint64_vec", __domainid (D), x, out);
    return out;
}

/** @}*/
/** \addtogroup <xor3pp_max_2>
 *  @{
 *  @brief Function for finding the pointwise maximum of 2 arrays
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @returns an array with the pointwise maximum of each element in the two input vectors
 *  @pre both input vectors are of equal length
 */

template <domain D : additive3pp>
D xor_uint8 max (D xor_uint8 x, D xor_uint8 y) {
    __syscall ("additive3pp::max_xor_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint16 max (D xor_uint16 x, D xor_uint16 y) {
    __syscall ("additive3pp::max_xor_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint32 max (D xor_uint32 x, D xor_uint32 y) {
    __syscall ("additive3pp::max_xor_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint64 max (D xor_uint64 x, D xor_uint64 y) {
    __syscall ("additive3pp::max_xor_uint64_vec", __domainid (D), x, y, x);
    return x;
}

template <domain D : additive3pp>
D xor_uint8[[1]] max (D xor_uint8[[1]] x, D xor_uint8[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::max_xor_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint16[[1]] max (D xor_uint16[[1]] x, D xor_uint16[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::max_xor_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint32[[1]] max (D xor_uint32[[1]] x, D xor_uint32[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::max_xor_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D xor_uint64[[1]] max (D xor_uint64[[1]] x, D xor_uint64[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::max_xor_uint64_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D xor_uint8[[N]] max (D xor_uint8[[N]] x, D xor_uint8[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::max_xor_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D xor_uint16[[N]] max (D xor_uint16[[N]] x, D xor_uint16[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::max_xor_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D xor_uint32[[N]] max (D xor_uint32[[N]] x, D xor_uint32[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::max_xor_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D xor_uint64[[N]] max (D xor_uint64[[N]] x, D xor_uint64[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::max_xor_uint64_vec", __domainid (D), x, y, x);
    return x;
}
/** @}*/
/** @}*/
/** @}*/
