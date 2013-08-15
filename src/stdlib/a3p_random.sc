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
module a3p_random;

import additive3pp;
import matrix;
/**
* \endcond
*/

/**
* @file
* \defgroup a3p_random a3p_random.sc
* \defgroup shuffle shuffle
* \defgroup shuffle1 shuffle
* \defgroup shuffle2 shuffle(key)
* \defgroup shufflerows1 shuffleRows
* \defgroup shufflerows2 shuffleRows(key)
* \defgroup shufflecols1 shuffleColumns
* \defgroup shufflecols2 shuffleColumns(key)
* \defgroup randomize randomize
*/

/** \addtogroup <a3p_random> 
*@{
* @brief Module with functions for randomizing values
*/

/** \addtogroup <shuffle> 
 *  @{
 *  @brief Functions for shuffling values
 *  @note **D** - additive3pp protection domain
 *  @note **T** - any \ref data_types "data" type
 */

/** \addtogroup <shuffle1> 
 *  @{
 *  @brief Function for shuffling values
 *  @note **D** - additive3pp protection domain
 *  @return returns a shuffled vector
 */

/**
* @note Supported types - \ref bool "bool"
* @param vec - a vector of supported type
*/
template <domain D : additive3pp>
D bool[[1]] shuffle (D bool[[1]] vec) {
    D uint8[[1]] vec_uint8 = (uint8) vec;
    __syscall ("additive3pp::vecshuf_uint8_vec", __domainid (D), vec_uint8);
    return (bool) vec_uint8;
}

/**
*  @note **T** - any \ref data_types "data" type
*  @param vec - a vector of supported type
*/
template <domain D : additive3pp, type T>
D T[[1]] shuffle (D T[[1]] vec) {
    __syscall ("additive3pp::vecshuf_$T\_vec", __domainid (D), vec);
    return vec;
}

/** @}*/
/** \addtogroup <shuffle2> 
 *  @{
 *  @brief Protocols to shuffle an array with given key.
 *  @note **D** - additive3pp protection domain
 *  @pre the key is exactly 32 bytes long
 *  @returns a random permutation of the input array
 *  @post the output is a permutation of the input
 *  @note the declassified value of the key does not matter, internally only the shares are used.
 *   If two vectors are shuffled using the same key the permutation applied is the same as long
 *   as the vectors are of the same length, and the key does not get reshared.
 */

/**
* @note Supported types - \ref bool "bool"
*  @param vec - boolean input array to shuffle
*  @param key - an \ref uint8 "uint8" type key that specifies the permutation
*/
template <domain D : additive3pp>
D bool[[1]] shuffle (D bool[[1]] vec, D uint8[[1]] key) {
    assert (size (key) == 32);
     D uint8[[1]] vec_uint8 = (uint8) vec;
    __syscall ("additive3pp::vecshufkey_uint8_vec", __domainid (D), vec_uint8, key);
    return (bool) vec_uint8;
}

/**
*  @note **T** - any \ref data_types "data" type
*  @param vec - input array to shuffle
*  @param key - an \ref uint8 "uint8" type key that specifies the permutation
*/
template <domain D : additive3pp, type T>
D T[[1]] shuffle (D T[[1]] vec, D uint8[[1]] key) {
    assert (size (key) == 32);
    __syscall ("additive3pp::vecshufkey_$T\_vec", __domainid (D), vec, key);
    return vec;
}
/** @}*/
/** \addtogroup <shufflerows1> 
*  @{
*  @brief Function for shuffling rows in a matrix
*  @note **D** - additive3pp protection domain
*  @return returns a matrix with shuffled rows
*/

/**
* @note Supported types - \ref bool "bool"
* @param mat - a matrix of type boolean
*/
template <domain D : additive3pp>
D bool[[2]] shuffleRows (D bool[[2]] mat) {
    D uint8[[2]] mat_uint8 = (uint8) mat;
    __syscall ("additive3pp::matshuf_uint8_vec", __domainid (D), mat_uint8, shape (mat)[1]);
    return (bool) mat_uint8;
}
/**
*  @note **T** - any \ref data_types "data" type
*  @param mat - a matrix of any type
*/
template <domain D : additive3pp, type T>
D T[[2]] shuffleRows (D T[[2]] mat) {
    __syscall ("additive3pp::matshuf_$T\_vec", __domainid (D), mat, shape (mat)[1]);
    return mat;
}

/** @}*/
/** \addtogroup <shufflerows2> 
 *  @{
 *  @brief Protocols to shuffle rows in a matrix with given key.
 *  @note **D** - additive3pp protection domain
 *  @pre the key is exactly 32 bytes long
 *  @returns a random permutation of the input matrix
 *  @post the output matrices rows are a permutation of the input
 *  @note the declassified value of the key does not matter, internally only the shares are used.
 *   If two vectors are shuffled using the same key the permutation applied is the same as long
 *   as the vectors are of the same length, and the key does not get reshared.
 */

/**
*  @note Supported types - \ref bool "bool"
*  @param mat - input matrix of type boolean to shuffle
*  @param key - an \ref uint8 "uint8" type key that specifies the permutation
*/
template <domain D : additive3pp>
D bool[[2]] shuffleRows (D bool[[2]] mat, D uint8[[1]] key) {
    assert (size (key) == 32);
    D uint8[[2]] mat_uint8 = (uint8) mat;
    __syscall ("additive3pp::matshufkey_uint8_vec", __domainid (D), mat_uint8, shape (mat)[1], key);
    return (bool) mat_uint8;
}
/**
*  @note **T** - any \ref data_types "data" type
*  @param mat - input matrix to shuffle
*  @param key - an \ref uint8 "uint8" type key that specifies the permutation
*/
template <domain D : additive3pp, type T>
D T[[2]] shuffleRows (D T[[2]] mat, D uint8[[1]] key) {
    assert (size (key) == 32);
    __syscall ("additive3pp::matshufkey_$T\_vec", __domainid (D), mat, shape (mat)[1], key);
    return mat;
}

/** @}*/
/** \addtogroup <shufflecols1> 
*  @{
*  @brief Function for shuffling columns in a matrix
*  @note **D** - additive3pp protection domain
*  @return returns a matrix with shuffled columns
*/

/**
* @note Supported types - \ref bool "bool"
* @param mat - a matrix of type boolean
*/
template <domain D : additive3pp>
D bool[[2]] shuffleColumns (D bool[[2]] mat) {
    return (bool)transpose(shuffleRows(transpose((uint8)mat)));
}
/**
*  @note **T** - any \ref data_types "data" type
*  @param mat - a matrix of any type
*/
template <domain D : additive3pp, type T>
D T[[2]] shuffleColumns (D T[[2]] mat) {
    return transpose(shuffleRows(transpose(mat)));
}


/** @}*/
/** \addtogroup <shufflecols2> 
 *  @{
 *  @brief Protocols to shuffle columns in a matrix with given key.
 *  @note **D** - additive3pp protection domain
 *  @pre the key is exactly 32 bytes long
 *  @returns a random permutation of the input matrix
 *  @post the output matrixes columns are a permutation of the input
 *  @note the declassified value of the key does not matter, internally only the shares are used.
 *   If two vectors are shuffled using the same key the permutation applied is the same as long
 *   as the vectors are of the same length, and the key does not get reshared.
 */

/**
*  @note Supported types - \ref bool "bool"
*  @param mat - input matrix of type boolean to shuffle
*  @param key - an \ref uint8 "uint8" type key that specifies the permutation
*/
template <domain D : additive3pp>
D bool[[2]] shuffleColumns (D bool[[2]] mat, D uint8[[1]] key) {
    assert (size (key) == 32);
    return (bool)transpose(shuffleRows(transpose((uint8)mat), key));
}

/**
*  @note **T** - any \ref data_types "data" type
*  @param mat - input matrix to shuffle
*  @param key - an \ref uint8 "uint8" type key that specifies the permutation
*/
template <domain D : additive3pp, type T>
D T[[2]] shuffleColumns (D T[[2]] mat, D uint8[[1]] key) {
    assert (size (key) == 32);
    return transpose(shuffleRows(transpose(mat), key));
}

/** @}*/
/** @}*/

/*******************************
    randomize
********************************/


/** \addtogroup <randomize> 
 *  @{
 *  @brief Function for randomizing values
 *  @note **D** - additive3pp protection domain
 *  @note **T** - any \ref data_types "data" type
 *  @param arr - an array of any dimension
 *  @return returns an array with randomized values
 */

template <domain D : additive3pp, dim N>
D bool[[N]] randomize(D bool[[N]] arr) {
    __syscall("additive3pp::randomize_bool_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint8[[N]] randomize(D uint8[[N]] arr) {
    __syscall("additive3pp::randomize_uint8_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint16[[N]] randomize(D uint16[[N]] arr) {
    __syscall("additive3pp::randomize_uint16_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint32[[N]] randomize(D uint32[[N]] arr) {
    __syscall("additive3pp::randomize_uint32_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint[[N]] randomize(D uint[[N]] arr) {
    __syscall("additive3pp::randomize_uint64_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D int8[[N]] randomize(D int8[[N]] arr) {
    __syscall("additive3pp::randomize_int8_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D int16[[N]] randomize(D int16[[N]] arr) {
    __syscall("additive3pp::randomize_int16_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D int32[[N]] randomize(D int32[[N]] arr) {
    __syscall("additive3pp::randomize_int32_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D int[[N]] randomize(D int[[N]] arr) {
    __syscall("additive3pp::randomize_int64_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D xor_uint8[[N]] randomize(D xor_uint8[[N]] arr) {
    __syscall("additive3pp::randomize_xor_uint8_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D xor_uint16[[N]] randomize(D xor_uint16[[N]] arr) {
    __syscall("additive3pp::randomize_xor_uint16_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D xor_uint32[[N]] randomize(D xor_uint32[[N]] arr) {
    __syscall("additive3pp::randomize_xor_uint32_vec", __domainid(D), arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D xor_uint64[[N]] randomize(D xor_uint64[[N]] arr) {
    __syscall("additive3pp::randomize_xor_uint64_vec", __domainid(D), arr);
    return arr;
}

/** @}*/
/** @}*/
