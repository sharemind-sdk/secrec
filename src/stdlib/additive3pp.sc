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

module additive3pp;

import stdlib;

kind additive3pp;
/**
* \endcond
*/
/**
* @file
* \defgroup additive3pp additive3pp.sc
* \defgroup sign sign
* \defgroup a3p_abs abs
* \defgroup a3p_sum sum
* \defgroup a3p_sum_vec sum
* \defgroup a3p_sum_k sum(parts)
* \defgroup a3p_product product
* \defgroup a3p_product_vec product
* \defgroup a3p_product_k product(parts)
* \defgroup a3p_any any
* \defgroup a3p_all all
* \defgroup a3p_trueprefixlength truePrefixLength
* \defgroup a3p_inv inv
* \defgroup a3p_sqrt sqrt
* \defgroup a3p_sin sin
* \defgroup a3p_ln ln
* \defgroup a3p_exp exp
* \defgroup a3p_erf erf
* \defgroup a3p_isnegligible isNegligible
* \defgroup a3p_min min
* \defgroup a3p_min_vec min
* \defgroup a3p_min_k min(parts)
* \defgroup a3p_min_2 min(2 vectors)
* \defgroup a3p_max max
* \defgroup a3p_max_vec max
* \defgroup a3p_max_k max(parts)
* \defgroup a3p_max_2 max(2 vectors)
* \defgroup a3p_floor floor
* \defgroup a3p_ceiling ceiling
*/

/** \addtogroup <additive3pp>
*@{
* @brief Module with additive3pp protection domain functions
*/



/*******************************
    classify
********************************/
/**
* \cond
*/
template <domain D : additive3pp, type T, dim N>
D T[[N]] classify(T[[N]] value) {
    D T[[N]] out;
    __syscall("additive3pp::classify_$T\_vec", __domainid(D), __cref value, out);
    return out;
}
/**
* \endcond
*/

/*******************************
    sign, abs
********************************/

/** \addtogroup <sign>
 *  @{
 *  @brief Function for determining the sign of values
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @param x - an array of any dimension
 *  @return returns an array of equal shape, size and dimension, where -1 signifies that, in the input array at that position was a negative number and 1  that it was a positive number
 */

template <domain D : additive3pp, dim N>
D int8[[N]] sign (D int8[[N]] x) {
    __syscall ("additive3pp::sign_int8_vec", __domainid (D), x, x);
    return x;
}

template <domain D : additive3pp, dim N>
D int16[[N]] sign (D int16[[N]] x) {
    __syscall ("additive3pp::sign_int16_vec", __domainid (D), x, x);
    return x;
}

template <domain D : additive3pp, dim N>
D int32[[N]] sign (D int32[[N]] x) {
    __syscall ("additive3pp::sign_int32_vec", __domainid (D), x, x);
    return x;
}

template <domain D : additive3pp, dim N>
D int[[N]] sign (D int[[N]] x) {
    __syscall ("additive3pp::sign_int64_vec", __domainid (D), x, x);
    return x;
}

/** @}*/
/** \addtogroup <a3p_abs>
 *  @{
 *  @brief Function for finding absolute values
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @param x - an array of any dimension
 *  @return returns an array of equal shape, size and dimension, where all values are the absolute values of the input array at that position
 */

template <domain D : additive3pp, dim N>
D uint8[[N]] abs (D int8[[N]] x) {
    D uint8[[N]] y;
    y = (uint8) x;
    __syscall ("additive3pp::abs_int8_vec", __domainid (D), x, y);
    return y;
}

template <domain D : additive3pp, dim N>
D uint16[[N]] abs (D int16[[N]] x) {
    D uint16[[N]] y;
    y = (uint16) x;
    __syscall ("additive3pp::abs_int16_vec", __domainid (D), x, y);
    return y;
}

template <domain D : additive3pp, dim N>
D uint32[[N]] abs (D int32[[N]] x) {
    D uint32[[N]] y;
    y = (uint32) x;
    __syscall ("additive3pp::abs_int32_vec", __domainid (D), x, y);
    return y;
}

template <domain D : additive3pp, dim N>
D uint[[N]] abs (D int[[N]] x) {
    D uint[[N]] y;
    y = (uint) x;
    __syscall ("additive3pp::abs_int64_vec", __domainid (D), x, y);
    return y;
}
/** @}*/

/*******************************
    sum
********************************/
/** \addtogroup <a3p_sum>
 *  @{
 *  @brief Functions for finding sums
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 */

/** \addtogroup <a3p_sum_vec>
 *  @{
 *  @brief Function for finding the sum of all the elements in a vector
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @note We are using a system call for summing vectors as it's very common
 *  operation, and the performance overhead of manually summing is in the
 *  range of 100 to 200 times slower.
 *  @param x - a 1-dimensional array
 *  @returns the sum of all input vector elements
 */


/**
* @note boolean values are converted to numerical values and then added, for more info click \ref bool "here"
* @return returns the sum of all the elements in the input vector as an \ref uint "uint64" type value
*/
template <domain D : additive3pp>
D uint sum (D bool[[1]] vec) {
    D uint out;
    __syscall ("additive3pp::sum_bool_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint8 sum (D uint8[[1]] vec) {
    D uint8 out;
    __syscall ("additive3pp::sum_uint8_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint16 sum (D uint16[[1]] vec) {
    D uint16 out;
    __syscall ("additive3pp::sum_uint16_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint32 sum (D uint32[[1]] vec) {
    D uint32 out;
    __syscall ("additive3pp::sum_uint32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint sum (D uint[[1]] vec) {
    D uint out;
    __syscall ("additive3pp::sum_uint64_vec", __domainid (D), vec, out);
    return out;
}


template <domain D : additive3pp>
D int8 sum (D int8[[1]] vec) {
    D int8 out;
    __syscall ("additive3pp::sum_int8_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int16 sum (D int16[[1]] vec) {
    D int16 out;
    __syscall ("additive3pp::sum_int16_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int32 sum (D int32[[1]] vec) {
    D int32 out;
    __syscall ("additive3pp::sum_int32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int sum (D int[[1]] vec) {
    D int out;
    __syscall ("additive3pp::sum_int64_vec", __domainid (D), vec, out);
    return out;
}


template <domain D : additive3pp>
D float32 sum (D float32[[1]] vec) {
    D float32 out;
    __syscall ("additive3pp::sum_float32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D float64 sum (D float64[[1]] vec) {
    D float64 out;
    __syscall ("additive3pp::sum_float64_vec", __domainid (D), vec, out);
    return out;
}

/** @}*/

/** \addtogroup <a3p_sum_k>
 *  @{
 *  @brief Function for finding the sum of all elements in the input vector in specified parts.
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @pre the length of the input array must be a multiple of **k**
 *  @param vec - The input array of subarrays to sum. The subarrays are stacked one after another and are all of the same size.
 *  @param k - The number of subarrays in the input array.
 *  @return The array of subarrayCount number of sums, each corresponding to respective subarray in the input array **vec**.
 */


template <domain D : additive3pp>
D uint[[1]] sum (D bool[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D uint[[1]] out (k);
    __syscall ("additive3pp::sum_bool_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint8[[1]] sum (D uint8[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D uint8[[1]] out (k);
    __syscall ("additive3pp::sum_uint8_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint16[[1]] sum (D uint16[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D uint16[[1]] out (k);
    __syscall ("additive3pp::sum_uint16_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint32[[1]] sum (D uint32[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D uint32[[1]] out (k);
    __syscall ("additive3pp::sum_uint32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint[[1]] sum (D uint[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D uint[[1]] out (k);
    __syscall ("additive3pp::sum_uint64_vec", __domainid (D), vec, out);
    return out;
}


template <domain D : additive3pp>
D int8[[1]] sum (D int8[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D int8[[1]] out (k);
    __syscall ("additive3pp::sum_int8_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int16[[1]] sum (D int16[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D int16[[1]] out (k);
    __syscall ("additive3pp::sum_int16_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int32[[1]] sum (D int32[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D int32[[1]] out (k);
    __syscall ("additive3pp::sum_int32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int[[1]] sum (D int[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D int[[1]] out (k);
    __syscall ("additive3pp::sum_int64_vec", __domainid (D), vec, out);
    return out;
}


template <domain D : additive3pp>
D float32[[1]] sum (D float32[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D float32[[1]] out (k);
    __syscall ("additive3pp::sum_float32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D float64[[1]] sum (D float64[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D float64[[1]] out (k);
    __syscall ("additive3pp::sum_float64_vec", __domainid (D), vec, out);
    return out;
}

/** @}*/
/** @}*/

/*******************************
    product
********************************/
/** \addtogroup <a3p_product>
 *  @{
 *  @brief Functions for finding products
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 */


/** \addtogroup <a3p_product_vec>
 *  @{
 *  @brief Function for finding the product of the input vector
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @param vec - a vector of supported type
 *  @return The product of the input vector
 */

template <domain D : additive3pp, type T>
D T product (D T scalar) {
    return scalar;
}

template <domain D : additive3pp>
D uint8 product (D uint8[[1]] vec) {
    D uint8 out;
    __syscall ("additive3pp::product_uint8_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint16 product (D uint16[[1]] vec) {
    D uint16 out;
    __syscall ("additive3pp::product_uint16_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint32 product (D uint32[[1]] vec) {
    D uint32 out;
    __syscall ("additive3pp::product_uint32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint product (D uint[[1]] vec) {
    D uint out;
    __syscall ("additive3pp::product_uint64_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int8 product (D int8[[1]] vec) {
    D int8 out;
    __syscall ("additive3pp::product_int8_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int16 product (D int16[[1]] vec) {
    D int16 out;
    __syscall ("additive3pp::product_int16_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int32 product (D int32[[1]] vec) {
    D int32 out;
    __syscall ("additive3pp::product_int32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int product (D int[[1]] vec) {
    D int out;
    __syscall ("additive3pp::product_int64_vec", __domainid (D), vec, out);
    return out;
}

/** @}*/
/** \addtogroup <a3p_product_k>
 *  @{
 *  @brief Function for finding the product of all elements in the input vector in specified parts.
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @pre the length of the input array must be a multiple of **k**
 *  @param vec - The input array of subarrays to find the product of. The subarrays are stacked one after another and are all of the same size.
 *  @param k - The number of subarrays in the input array.
 *  @return The array of subarrayCount number of products, each corresponding to respective subarray in the input array **vec**.
 */

template <domain D : additive3pp>
D uint8[[1]] product (D uint8[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D uint8[[1]] out (k);
    __syscall ("additive3pp::product_uint8_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint16[[1]] product (D uint16[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D uint16[[1]] out (k);
    __syscall ("additive3pp::product_uint16_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint32[[1]] product (D uint32[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D uint32[[1]] out (k);
    __syscall ("additive3pp::product_uint32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D uint[[1]] product (D uint[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D uint[[1]] out (k);
    __syscall ("additive3pp::product_uint64_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int8[[1]] product (D int8[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D int8[[1]] out (k);
    __syscall ("additive3pp::product_int8_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int16[[1]] product (D int16[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D int16[[1]] out (k);
    __syscall ("additive3pp::product_int16_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int32[[1]] product (D int32[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D int32[[1]] out (k);
    __syscall ("additive3pp::product_int32_vec", __domainid (D), vec, out);
    return out;
}

template <domain D : additive3pp>
D int[[1]] product (D int[[1]] vec, uint k) {
    assert(k > 0 && size(vec) % k == 0);
    D int[[1]] out (k);
    __syscall ("additive3pp::product_int64_vec", __domainid (D), vec, out);
    return out;
}

/** @}*/
/** @}*/


/*******************************
    any, all
********************************/


/** \addtogroup <a3p_any>
 *  @{
 *  @brief Function for checking if any value of the input vector is true
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool"
 *  @return true if any of the input bits is set
 *  @return false if all input bits are not set
 *  @note performs one vectorized cast, and one comparison against zero
 */

/**
* @param b - scalar boolean
*/
template <domain D : additive3pp>
D bool any (D bool b) {
    return b;
}

/**
* @param vec - boolean 1-dimensional vector
*/
template <domain D : additive3pp>
D bool any (D bool[[1]] vec) {
    return sum (vec) != 0;
}

/**
* @param vec - boolean 1-dimensional vector
* @param k - an \ref uint64 "uint" type value that shows in how many subarrays must **any** be found
*/
template <domain D : additive3pp>
D bool[[1]] any (D bool[[1]] vec, uint k) {
    return sum (vec, k) != 0;
}

/**
* @param arr - boolean any dimension vector
*/
template <domain D : additive3pp, dim N>
D bool any (D bool[[N]] arr) {
    return any(flatten(arr));
}
/** @}*/

/** \addtogroup <a3p_all>
 *  @{
 *  @brief Function for checking if all values of the input vector are true
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool"
 *  @return true if all of the input bits are set
 *  @return false if any input bit is not set
 *  @note performs one vectorized cast, and one comparison against length of the vector
 */

/**
* @param b - scalar boolean
*/
template <domain D : additive3pp>
D bool all (D bool b) {
    return b;
}

/**
* @param vec - boolean 1-dimensional vector
*/
template <domain D : additive3pp>
D bool all (D bool [[1]] arr) {
    uint n = size (arr);
    return sum (arr) == n;
}

/**
* @param vec - boolean 1-dimensional vector
* @param k - an \ref uint64 "uint" type value that shows in how many subarrays must **all** be found
*/
template <domain D : additive3pp>
D bool[[1]] all (D bool[[1]] vec, uint k) {
    uint n = size(vec);
    assert(k > 0 && n % k == 0);
    uint len = n / k;
    return sum (vec, k) == len;
}

/**
* @param arr - boolean any dimension vector
*/
template <domain D : additive3pp, dim N>
D bool all (D bool[[N]] arr) {
    return all(flatten(arr));
}
/** @}*/

/*******************************
    truePrefixLength
********************************/

/** \addtogroup <a3p_trueprefixlength>
 *  @{
 *  @brief Function for finding how many elements from the start of a vector are true
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool"
 *  @returns the number of set bits in the longest constant true prefix of the input
 *  @note this function performs log n multiplications on vectors of at most size n this is more efficient than performing n multiplications on scalars
 * \todo i think this can be further optimized
 */

/**
* @param arr - boolean 1-dimensional vector
*/
template <domain D : additive3pp>
D uint truePrefixLength (D bool [[1]] arr) {
    for (uint shift = 1, n = size (arr); shift < n; shift *= 2) {
        arr[shift:] = arr[shift:] & arr[:n-shift];
    }

    return sum (arr);
}

/** @}*/

/*******************************
    mulc
********************************/

/**
* \cond
*/

template <domain D : additive3pp>
D uint8 operator * (D uint8 a, uint8 b) {
    __syscall ("additive3pp::mulc_uint8_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D uint8[[1]] operator * (D uint8[[1]] a, uint8[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_uint8_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D uint8[[N]] operator * (D uint8[[N]] arr, uint8 pubScalar) {
    uint8[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_uint8_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint8[[N]] operator * (D uint8[[N]] arr, uint8[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_uint8_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D uint16 operator * (D uint16 a, uint16 b) {
    __syscall ("additive3pp::mulc_uint16_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D uint16[[1]] operator * (D uint16[[1]] a, uint16[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_uint16_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D uint16[[N]] operator * (D uint16[[N]] arr, uint16 pubScalar) {
    uint16[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_uint16_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint16[[N]] operator * (D uint16[[N]] arr, uint16[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_uint16_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D uint32 operator * (D uint32 a, uint32 b) {
    __syscall ("additive3pp::mulc_uint32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D uint32[[1]] operator * (D uint32[[1]] a, uint32[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_uint32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D uint32[[N]] operator * (D uint32[[N]] arr, uint32 pubScalar) {
    uint32[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_uint32_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint32[[N]] operator * (D uint32[[N]] arr, uint32[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_uint32_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D uint operator * (D uint a, uint b) {
    __syscall ("additive3pp::mulc_uint64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D uint[[1]] operator * (D uint[[1]] a, uint[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_uint64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D uint[[N]] operator * (D uint[[N]] arr, uint pubScalar) {
    uint[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_uint64_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint[[N]] operator * (D uint[[N]] arr, uint[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_uint64_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D int8 operator * (D int8 a, int8 b) {
    __syscall ("additive3pp::mulc_int8_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D int8[[1]] operator * (D int8[[1]] a, int8[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_int8_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D int8[[N]] operator * (D int8[[N]] arr, int8 pubScalar) {
    int8[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_int8_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D int8[[N]] operator * (D int8[[N]] arr, int8[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_int8_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D int16 operator * (D int16 a, int16 b) {
    __syscall ("additive3pp::mulc_int16_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D int16[[1]] operator * (D int16[[1]] a, int16[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_int16_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D int16[[N]] operator * (D int16[[N]] arr, int16 pubScalar) {
    int16[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_int16_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D int16[[N]] operator * (D int16[[N]] arr, int16[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_int16_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D int32 operator * (D int32 a, int32 b) {
    __syscall ("additive3pp::mulc_int32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D int32[[1]] operator * (D int32[[1]] a, int32[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_int32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D int32[[N]] operator * (D int32[[N]] arr, int32 pubScalar) {
    int32[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_int32_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D int32[[N]] operator * (D int32[[N]] arr, int32[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_int32_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D int operator * (D int a, int b) {
    __syscall ("additive3pp::mulc_int64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D int[[1]] operator * (D int[[1]] a, int[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_int64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D int[[N]] operator * (D int[[N]] arr, int pubScalar) {
    int[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_int64_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D int[[N]] operator * (D int[[N]] arr, int[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_int64_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D float32 operator * (D float32 a, float32 b) {
    __syscall ("additive3pp::mulc_float32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D float32[[1]] operator * (D float32[[1]] a, float32[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_float32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}


template <domain D : additive3pp, dim N>
D float32[[N]] operator * (D float32[[N]] arr, float32 pubScalar) {
    float32[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_float32_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D float32[[N]] operator * (D float32[[N]] arr, float32[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_float32_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D float64 operator * (D float64 a, float64 b) {
    __syscall ("additive3pp::mulc_float64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D float64[[1]] operator * (D float64[[1]] a, float64[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::mulc_float64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D float64[[N]] operator * (D float64[[N]] arr, float64 pubScalar) {
    float64[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::mulc_float64_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D float64[[N]] operator * (D float64[[N]] arr, float64[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::mulc_float64_vec", __domainid (D),
        arr, __cref pubArr, arr);
    return arr;
}


/*******************************
    divc
********************************/

// Divc on uint8 and uint16 is unstable and produces wrong results. Worst case scenario results in a total crash of miners.
/*
template <domain D : additive3pp>
D uint8 operator / (D uint8 a, uint8 b) {
    __syscall ("additive3pp::divc_uint8_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D uint8[[1]] operator / (D uint8[[1]] a, uint8[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::divc_uint8_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D uint8[[N]] operator / (D uint8[[N]] arr, uint8 pubScalar) {
    uint8[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::divc_uint8_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint8[[N]] operator / (D uint8[[N]] arr, uint8[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::divc_uint8_vec", __domainid (D),
       arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D uint16 operator / (D uint16 a, uint16 b) {
    __syscall ("additive3pp::divc_uint16_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D uint16[[1]] operator / (D uint16[[1]] a, uint16[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::divc_uint16_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D uint16[[N]] operator / (D uint16[[N]] arr, uint16 pubScalar) {
    uint16[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::divc_uint16_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint16[[N]] operator / (D uint16[[N]] arr, uint16[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::divc_uint16_vec", __domainid (D),
       arr, __cref pubArr, arr);
    return arr;
}
*/

template <domain D : additive3pp>
D uint32 operator / (D uint32 a, uint32 b) {
    __syscall ("additive3pp::divc_uint32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D uint32[[1]] operator / (D uint32[[1]] a, uint32[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::divc_uint32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D uint32[[N]] operator / (D uint32[[N]] arr, uint32 pubScalar) {
    uint32[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::divc_uint32_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint32[[N]] operator / (D uint32[[N]] arr, uint32[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::divc_uint32_vec", __domainid (D),
       arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D uint operator / (D uint a, uint b) {
    __syscall ("additive3pp::divc_uint64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D uint[[1]] operator / (D uint[[1]] a, uint[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::divc_uint64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D uint[[N]] operator / (D uint[[N]] arr, uint pubScalar) {
    uint[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::divc_uint64_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D uint[[N]] operator / (D uint[[N]] arr, uint[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::divc_uint64_vec", __domainid (D),
       arr, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D float32 operator / (D float32 a, float32 b) {
    __syscall ("additive3pp::divc_float32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D float32[[1]] operator / (D float32[[1]] a, float32[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::divc_float32_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D float32[[N]] operator / (D float32[[N]] arr, float32 pubScalar) {
    float32[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::divc_float32_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D float32[[N]] operator / (D float32[[N]] arr, float32[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::divc_float32_vec", __domainid (D),
        a, __cref pubArr, arr);
    return arr;
}

template <domain D : additive3pp>
D float64 operator / (D float64 a, float64 b) {
    __syscall ("additive3pp::divc_float64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp>
D float64[[1]] operator / (D float64[[1]] a, float64[[1]] b) {
    assert(size(a) == size(b));
    __syscall ("additive3pp::divc_float64_vec", __domainid (D),
        a, __cref b, a);
    return a;
}

template <domain D : additive3pp, dim N>
D float64[[N]] operator / (D float64[[N]] arr, float64 pubScalar) {
    float64[[1]] pubVec (size(arr)) = pubScalar;
    __syscall ("additive3pp::divc_float64_vec", __domainid (D),
        arr, __cref pubVec, arr);
    return arr;
}

template <domain D : additive3pp, dim N>
D float64[[N]] operator / (D float64[[N]] arr, float64[[N]] pubArr) {
    assert(shapesAreEqual(arr,pubArr));
    __syscall ("additive3pp::divc_float64_vec", __domainid (D),
        a, __cref pubArr, arr);
    return arr;
}

/**
* \endcond
*/

/*****************************************************
    inv, sqrt, sin, ln, exp, erf, isNegligible
*****************************************************/

/** \addtogroup <a3p_inv>
 *  @{
 *  @brief Function for inversing a value
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @return returns the inversed values of the input array
 */

template <domain D : additive3pp, dim N>
D float32[[N]] inv (D float32[[N]] x) {
    __syscall ("additive3pp::inv_float32_vec", __domainid (D), x, x);
    return x;
}

template <domain D : additive3pp, dim N>
D float64[[N]] inv (D float64[[N]] x) {
    __syscall ("additive3pp::inv_float64_vec", __domainid (D), x, x);
    return x;
}

/** @}*/
/** \addtogroup <a3p_sqrt>
 *  @{
 *  @brief Function for finding the square root of a value
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @return returns the square roots of the input array
 */

template <domain D : additive3pp, dim N>
D float32[[N]] sqrt (D float32[[N]] x) {
    __syscall ("additive3pp::sqrt_float32_vec", __domainid (D), x, x);
    return x;
}

template <domain D : additive3pp, dim N>
D float64[[N]] sqrt (D float64[[N]] x) {
    __syscall ("additive3pp::sqrt_float64_vec", __domainid (D), x, x);
    return x;
}

/** @}*/
/** \addtogroup <a3p_sin>
 *  @{
 *  @brief Function for finding the sine of a value
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @return returns the sines of the input array
 */

template <domain D : additive3pp, dim N>
D float32[[N]] sin (D float32[[N]] x) {
    __syscall ("additive3pp::sin_float32_vec", __domainid (D), x, x);
    return x;
}

template <domain D : additive3pp, dim N>
D float64[[N]] sin (D float64[[N]] x) {
    __syscall ("additive3pp::sin_float64_vec", __domainid (D), x, x);
    return x;
}

/** @}*/
/** \addtogroup <a3p_ln>
 *  @{
 *  @brief Function for finding the natural logarithm of a value
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @return returns the natural logarithms of the input array
 */

template <domain D : additive3pp, dim N>
D float32[[N]] ln (D float32[[N]] x) {
    __syscall ("additive3pp::ln_float32_vec", __domainid (D), x, x);
    return x;
}

template <domain D : additive3pp, dim N>
D float64[[N]] ln (D float64[[N]] x) {
    __syscall ("additive3pp::ln_float64_vec", __domainid (D), x, x);
    return x;
}

/** @}*/
/** \addtogroup <a3p_exp>
 *  @{
 *  @brief Function for finding exp(x)
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @return returns the exponents of the input array
 */

template <domain D : additive3pp, dim N>
D float32[[N]] exp (D float32[[N]] x) {
    __syscall ("additive3pp::exp_float32_vec", __domainid (D), x, x);
    return x;
}

template <domain D : additive3pp, dim N>
D float64[[N]] exp (D float64[[N]] x) {
    __syscall ("additive3pp::exp_float64_vec", __domainid (D), x, x);
    return x;
}

/** @}*/
/** \addtogroup <a3p_erf>
 *  @{
 *  @brief Function for finding the value of error function
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @return returns the error functions of the input array
 */

template <domain D : additive3pp, dim N>
D float32[[N]] erf (D float32[[N]] x) {
    __syscall ("additive3pp::erf_float32_vec", __domainid (D), x, x);
    return x;
}

template <domain D : additive3pp, dim N>
D float64[[N]] erf (D float64[[N]] x) {
    __syscall ("additive3pp::erf_float64_vec", __domainid (D), x, x);
    return x;
}

/** @}*/
/** \addtogroup <a3p_isnegligible>
 *  @{
 *  @brief Function for finding if the error is small enough to neglect
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @return returns **true** if the error is small enough to neglect
 *  @return returns **false** if the error is not small enough
 *  @note isNegligible checks up to the 5th place after the comma
 */

/**
* @param a - a scalar of supported type
* @return returns **true** if the error is small enough to neglect
* @return returns **false** if the error is not small enough
*/
template <domain D : additive3pp>
D bool isNegligible (D float32 a) {
    D bool out;
    __syscall ("additive3pp::isnegligible_float32_vec", __domainid (D), a, out);
    return out;
}

/**
* @param a - a scalar of supported type
* @return returns **true** if the error is small enough to neglect
* @return returns **false** if the error is not small enough
*/
template <domain D : additive3pp>
D bool isNegligible (D float64 a) {
    D bool out;
    __syscall ("additive3pp::isnegligible_float64_vec", __domainid (D), a, out);
    return out;
}

/**
* @param a - a vector of supported type
* @return returns a vector where each element of the input vector has been evaluated, whether the error is small enough to neglect or not
*/
template <domain D : additive3pp>
D bool[[1]] isNegligible (D float32[[1]] a) {
    D bool[[1]] out (size (a));
    __syscall ("additive3pp::isnegligible_float32_vec", __domainid (D), a, out);
    return out;
}

/**
* @param a - a vector of supported type
* @return returns a vector where each element of the input vector has been evaluated, whether the error is small enough to neglect or not
*/
template <domain D : additive3pp>
D bool[[1]] isNegligible (D float64[[1]] a) {
    D bool[[1]] out (size (a));
    __syscall ("additive3pp::isnegligible_float64_vec", __domainid (D), a, out);
    return out;
}
/** @}*/

/*******************************
    Min, max
********************************/

/** \addtogroup <a3p_min>
 *  @{
 *  @brief Functions for finding the minimum value
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 */

/** \addtogroup <a3p_min_vec>
 *  @{
 *  @brief Function for finding the minimum element of the input vector.
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @returns minimum element of the input vector.
 *  @pre input vector is not empty
 */

template <domain D : additive3pp, type T>
D T min (D T scalar) {
    return scalar;
}

template <domain D : additive3pp>
D uint8 min (D uint8[[1]] x) {
    assert (size(x) > 0);
    D uint8 out;
    __syscall ("additive3pp::vecmin_uint8_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D uint16 min (D uint16[[1]] x) {
    assert (size(x) > 0);
    D uint16 out;
    __syscall ("additive3pp::vecmin_uint16_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D uint32 min (D uint32[[1]] x) {
    assert (size(x) > 0);
    D uint32 out;
    __syscall ("additive3pp::vecmin_uint32_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D uint min (D uint[[1]] x) {
    assert (size(x) > 0);
    D uint out;
    __syscall ("additive3pp::vecmin_uint64_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D int8 min (D int8[[1]] x) {
    assert (size(x) > 0);
    D uint8[[1]] in = (uint8) x + 128;
    D uint8 out;
    __syscall ("additive3pp::vecmin_uint8_vec", __domainid (D), in, out);
    out -= 128;
    return (int8) out;
}
template <domain D : additive3pp>
D int16 min (D int16[[1]] x) {
    assert (size(x) > 0);
    D uint16[[1]] in = (uint16) x + 32768;
    D uint16 out;
    __syscall ("additive3pp::vecmin_uint16_vec", __domainid (D), in, out);
    out -= 32768;
    return (int16)out;
}
template <domain D : additive3pp>
D int32 min (D int32[[1]] x) {
    assert (size(x) > 0);
    D uint32[[1]] in = (uint32) x + 2147483648;
    D uint32 out;
    __syscall ("additive3pp::vecmin_uint32_vec", __domainid (D), in, out);
    out -= 2147483648;
    return (int32)out;
}
template <domain D : additive3pp>
D int min (D int[[1]] x) {
    assert (size(x) > 0);
    D uint[[1]] in = (uint) x + 9223372036854775808;
    D uint out;
    __syscall ("additive3pp::vecmin_uint64_vec", __domainid (D), in, out);
    out -= 9223372036854775808;
    return (int)out;
}

/** @}*/
/** \addtogroup <a3p_min_k>
 *  @{
 *  @brief Function for finding the minimum element in the specified parts of the vector.
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @param k - an \ref uint64 "uint" type value, which specifies into how many subarrays must the input array be divided
 *  @returns a vector with all the minimum elements of all the subarrays specified by k
 *  @pre input vector is not empty
 *  @pre the size of the input array is dividable by **k**
 */


template <domain D : additive3pp>
D uint8[[1]] min (D uint8[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint8[[1]] out (k);
    __syscall ("additive3pp::vecmin_uint8_vec", __domainid (D), x, out);
    return out;
}



template <domain D : additive3pp>
D uint16[[1]] min (D uint16[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint16[[1]] out (k);
    __syscall ("additive3pp::vecmin_uint16_vec", __domainid (D), x, out);
    return out;
}



template <domain D : additive3pp>
D uint32[[1]] min (D uint32[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint32[[1]] out (k);
    __syscall ("additive3pp::vecmin_uint32_vec", __domainid (D), x, out);
    return out;
}



template <domain D : additive3pp>
D uint[[1]] min (D uint[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint[[1]] out (k);
    __syscall ("additive3pp::vecmin_uint64_vec", __domainid (D), x, out);
    return out;
}


template <domain D : additive3pp>
D int8[[1]] min (D int8[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint8[[1]] in = (uint8) x + 128;
    D uint8[[1]] out (k);
    __syscall ("additive3pp::vecmin_uint8_vec", __domainid (D), in, out);
    out -= 128;
    return (int8) out;
}
template <domain D : additive3pp>
D int16[[1]] min (D int16[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint16[[1]] in = (uint16) x + 32768;
    D uint16[[1]] out (k);
    __syscall ("additive3pp::vecmin_uint16_vec", __domainid (D), in, out);
    out -= 32768;
    return (int16)out;
}
template <domain D : additive3pp>
D int32[[1]] min (D int32[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint32[[1]] in = (uint32) x + 2147483648;
    D uint32[[1]] out (k);
    __syscall ("additive3pp::vecmin_uint32_vec", __domainid (D), in, out);
    out -= 2147483648;
    return (int32)out;
}
template <domain D : additive3pp>
D int[[1]] min (D int[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint[[1]] in = (uint) x + 9223372036854775808;
    D uint[[1]] out (k);
    __syscall ("additive3pp::vecmin_uint64_vec", __domainid (D), in, out);
    out -= 9223372036854775808;
    return (int)out;
}

/** @}*/
/** \addtogroup <a3p_min_2>
 *  @{
 *  @brief Function for finding the pointwise minimum of 2 arrays
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @returns an array with the pointwise minimum of each element in the two input vectors
 *  @pre both input vectors are of equal length
 */

template <domain D : additive3pp>
D uint8 min (D uint8 x, D uint8 y) {
    __syscall ("additive3pp::min_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint16 min (D uint16 x, D uint16 y) {
    __syscall ("additive3pp::min_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint32 min (D uint32 x, D uint32 y) {
    __syscall ("additive3pp::min_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint min (D uint x, D uint y) {
    __syscall ("additive3pp::min_uint64_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D int8 min (D int8 x, D int8 y) {
    D uint8 in1 = (uint8) x + 128;
    D uint8 in2 = (uint8) y + 128;
    __syscall ("additive3pp::min_uint8_vec", __domainid (D), in1, in2, in1);
    in1 -= 128;
    return (int8)in1;
}
template <domain D : additive3pp>
D int16 min (D int16 x, D int16 y) {
    D uint16 in1 = (uint16) x + 32768;
    D uint16 in2 = (uint16) y + 32768;
    __syscall ("additive3pp::min_uint16_vec", __domainid (D), in1, in2, in1);
    in1 -= 32768;
    return (int16)in1;
}
template <domain D : additive3pp>
D int32 min (D int32 x, D int32 y) {
    D uint32 in1 = (uint32) x + 2147483648;
    D uint32 in2 = (uint32) y + 2147483648;
    __syscall ("additive3pp::min_uint32_vec", __domainid (D), in1, in2, in1);
    in1 -= 2147483648;
    return (int32)in1;
}
template <domain D : additive3pp>
D int min (D int x, D int y) {
    D uint in1 = (uint) x + 9223372036854775808;
    D uint in2 = (uint) y + 9223372036854775808;
    __syscall ("additive3pp::min_uint64_vec", __domainid (D), in1, in2, in1);
    in1 -= 9223372036854775808;
    return (int)in1;
}

template <domain D : additive3pp>
D uint8[[1]] min (D uint8[[1]] x, D uint8[[1]] y) {
    assert (size(x) == size(y));
    __syscall ("additive3pp::min_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint16[[1]] min (D uint16[[1]] x, D uint16[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::min_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint32[[1]] min (D uint32[[1]] x, D uint32[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::min_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint[[1]] min (D uint[[1]] x, D uint[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::min_uint64_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D int8[[1]] min (D int8[[1]] x, D int8[[1]] y) {
    assert (size (x) == size (y));
    D uint8[[1]] in1 = (uint8) x + 128;
    D uint8[[1]] in2 = (uint8) y + 128;
    __syscall ("additive3pp::min_uint8_vec", __domainid (D), in1, in2, in1);
    in1 -= 128;
    return (int8)in1;
}
template <domain D : additive3pp>
D int16[[1]] min (D int16[[1]] x, D int16[[1]] y) {
    assert (size (x) == size (y));
    D uint16[[1]] in1 = (uint16) x + 32768;
    D uint16[[1]] in2 = (uint16) y + 32768;
    __syscall ("additive3pp::min_uint16_vec", __domainid (D), in1, in2, in1);
    in1 -= 32768;
    return (int16)in1;
}
template <domain D : additive3pp>
D int32[[1]] min (D int32[[1]] x, D int32[[1]] y) {
    assert (size (x) == size (y));
    D uint32[[1]] in1 = (uint32) x + 2147483648;
    D uint32[[1]] in2 = (uint32) y + 2147483648;
    __syscall ("additive3pp::min_uint32_vec", __domainid (D), in1, in2, in1);
    in1 -= 2147483648;
    return (int32)in1;
}
template <domain D : additive3pp>
D int[[1]] min (D int[[1]] x, D int[[1]] y) {
    assert (size (x) == size (y));
    D uint[[1]] in1 = (uint) x + 9223372036854775808;
    D uint[[1]] in2 = (uint) y + 9223372036854775808;
    __syscall ("additive3pp::min_uint64_vec", __domainid (D), in1, in2, in1);
    in1 -= 9223372036854775808;
    return (int)in1;
}

template <domain D : additive3pp, dim N>
D uint8[[N]] min (D uint8[[N]] x, D uint8[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::min_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D uint16[[N]] min (D uint16[[N]] x, D uint16[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::min_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D uint32[[N]] min (D uint32[[N]] x, D uint32[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::min_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D uint[[N]] min (D uint[[N]] x, D uint[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::min_uint64_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D int8[[N]] min (D int8[[N]] x, D int8[[N]] y) {
    assert(shapesAreEqual(x,y));
    D uint8[[N]] in1 = (uint8) x + 128;
    D uint8[[N]] in2 = (uint8) y + 128;
    __syscall ("additive3pp::min_uint8_vec", __domainid (D), in1, in2, in1);
    in1 -= 128;
    return (int8)in1;
}
template <domain D : additive3pp, dim N>
D int16[[N]] min (D int16[[N]] x, D int16[[N]] y) {
    assert(shapesAreEqual(x,y));
    D uint16[[N]] in1 = (uint16) x + 32768;
    D uint16[[N]] in2 = (uint16) y + 32768;
    __syscall ("additive3pp::min_uint16_vec", __domainid (D), in1, in2, in1);
    in1 -= 32768;
    return (int16)in1;
}
template <domain D : additive3pp, dim N>
D int32[[N]] min (D int32[[N]] x, D int32[[N]] y) {
    assert(shapesAreEqual(x,y));
    D uint32[[N]] in1 = (uint32) x + 2147483648;
    D uint32[[N]] in2 = (uint32) y + 2147483648;
    __syscall ("additive3pp::min_uint32_vec", __domainid (D), in1, in2, in1);
    in1 -= 2147483648;
    return (int32)in1;
}
template <domain D : additive3pp, dim N>
D int[[N]] min (D int[[N]] x, D int[[N]] y) {
    assert(shapesAreEqual(x,y));
    D uint[[N]] in1 = (uint) x + 9223372036854775808;
    D uint[[N]] in2 = (uint) y + 9223372036854775808;
    __syscall ("additive3pp::min_uint64_vec", __domainid (D), in1, in2, in1);
    in1 -= 9223372036854775808;
    return (int)in1;
}

/** @}*/
/** @}*/


/** \addtogroup <a3p_max>
 *  @{
 *  @brief Functions for finding the maximum value
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 */

/** \addtogroup <a3p_max_vec>
 *  @{
 *  @brief Function for finding the maximum element of the input vector.
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @returns maximum element of the input vector.
 *  @pre input vector is not empty
 */

template <domain D : additive3pp, type T>
D T max (D T scalar) {
    return scalar;
}
template <domain D : additive3pp>
D uint8 max (D uint8[[1]] x) {
    assert (size(x) > 0);
    D uint8 out;
    __syscall ("additive3pp::vecmax_uint8_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D uint16 max (D uint16[[1]] x) {
    assert (size(x) > 0);
    D uint16 out;
    __syscall ("additive3pp::vecmax_uint16_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D uint32 max (D uint32[[1]] x) {
    assert (size(x) > 0);
    D uint32 out;
    __syscall ("additive3pp::vecmax_uint32_vec", __domainid (D), x, out);
    return out;
}
template <domain D : additive3pp>
D uint max (D uint[[1]] x) {
    assert (size(x) > 0);
    D uint out;
    __syscall ("additive3pp::vecmax_uint64_vec", __domainid (D), x, out);
    return out;
}

template <domain D : additive3pp>
D int8 max (D int8[[1]] x) {
    assert (size(x) > 0);
    D uint8[[1]] in = (uint8) x + 128;
    D uint8 out;
    __syscall ("additive3pp::vecmax_uint8_vec", __domainid (D), in, out);
    out -= 128;
    return (int8) out;
}
template <domain D : additive3pp>
D int16 max (D int16[[1]] x) {
    assert (size(x) > 0);
    D uint16[[1]] in = (uint16) x + 32768;
    D uint16 out;
    __syscall ("additive3pp::vecmax_uint16_vec", __domainid (D), in, out);
    out -= 32768;
    return (int16)out;
}
template <domain D : additive3pp>
D int32 max (D int32[[1]] x) {
    assert (size(x) > 0);
    D uint32[[1]] in = (uint32) x + 2147483648;
    D uint32 out;
    __syscall ("additive3pp::vecmax_uint32_vec", __domainid (D), in, out);
    out -= 2147483648;
    return (int32)out;
}
template <domain D : additive3pp>
D int max (D int[[1]] x) {
    assert (size(x) > 0);
    D uint[[1]] in = (uint) x + 9223372036854775808;
    D uint out;
    __syscall ("additive3pp::vecmax_uint64_vec", __domainid (D), in, out);
    out -= 9223372036854775808;
    return (int)out;
}

/** @}*/
/** \addtogroup <a3p_max_k>
 *  @{
 *  @brief Function for finding the maximum element in the specified parts of the vector.
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @param k - an \ref uint64 "uint" type value, which specifies into how many subarrays must the input array be divided
 *  @returns a vector with all the maximum elements of all the subarrays specified by k
 *  @pre input vector is not empty
 *  @pre the size of the input array is dividable by **k**
 */



template <domain D : additive3pp>
D uint8[[1]] max (D uint8[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint8[[1]] out (k);
    __syscall ("additive3pp::vecmax_uint8_vec", __domainid (D), x, out);
    return out;
}



template <domain D : additive3pp>
D uint16[[1]] max (D uint16[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint16[[1]] out (k);
    __syscall ("additive3pp::vecmax_uint16_vec", __domainid (D), x, out);
    return out;
}



template <domain D : additive3pp>
D uint32[[1]] max (D uint32[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint32[[1]] out (k);
    __syscall ("additive3pp::vecmax_uint32_vec", __domainid (D), x, out);
    return out;
}



template <domain D : additive3pp>
D uint[[1]] max (D uint[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint[[1]] out (k);
    __syscall ("additive3pp::vecmax_uint64_vec", __domainid (D), x, out);
    return out;
}


template <domain D : additive3pp>
D int8[[1]] max (D int8[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint8[[1]] in = (uint8) x + 128;
    D uint8[[1]] out (k);
    __syscall ("additive3pp::vecmax_uint8_vec", __domainid (D), in, out);
    out -= 128;
    return (int8) out;
}
template <domain D : additive3pp>
D int16[[1]] max (D int16[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint16[[1]] in = (uint16) x + 32768;
    D uint16[[1]] out (k);
    __syscall ("additive3pp::vecmax_uint16_vec", __domainid (D), in, out);
    out -= 32768;
    return (int16)out;
}
template <domain D : additive3pp>
D int32[[1]] max (D int32[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint32[[1]] in = (uint32) x + 2147483648;
    D uint32[[1]] out (k);
    __syscall ("additive3pp::vecmax_uint32_vec", __domainid (D), in, out);
    out -= 2147483648;
    return (int32)out;
}
template <domain D : additive3pp>
D int[[1]] max (D int[[1]] x, uint k) {
    uint n = size(x);
    assert(n > 0 && k > 0 && n % k == 0);
    D uint[[1]] in = (uint) x + 9223372036854775808;
    D uint[[1]] out (k);
    __syscall ("additive3pp::vecmax_uint64_vec", __domainid (D), in, out);
    out -= 9223372036854775808;
    return (int)out;
}

/** @}*/
/** \addtogroup <a3p_max_2>
 *  @{
 *  @brief Function for finding the pointwise maximum of 2 arrays
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @returns an array with the pointwise maximum of each element in the two input vectors
 *  @pre both input vectors are of equal length
 */

template <domain D : additive3pp>
D uint8 max (D uint8 x, D uint8 y) {
    __syscall ("additive3pp::max_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint16 max (D uint16 x, D uint16 y) {
    __syscall ("additive3pp::max_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint32 max (D uint32 x, D uint32 y) {
    __syscall ("additive3pp::max_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint max (D uint x, D uint y) {
    __syscall ("additive3pp::max_uint64_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D int8 max (D int8 x, D int8 y) {
    D uint8 in1 = (uint8) x + 128;
    D uint8 in2 = (uint8) y + 128;
    __syscall ("additive3pp::max_uint8_vec", __domainid (D), in1, in2, in1);
    in1 -= 128;
    return (int8)in1;
}
template <domain D : additive3pp>
D int16 max (D int16 x, D int16 y) {
    D uint16 in1 = (uint16) x + 32768;
    D uint16 in2 = (uint16) y + 32768;
    __syscall ("additive3pp::max_uint16_vec", __domainid (D), in1, in2, in1);
    in1 -= 32768;
    return (int16)in1;
}
template <domain D : additive3pp>
D int32 max (D int32 x, D int32 y) {
    D uint32 in1 = (uint32) x + 2147483648;
    D uint32 in2 = (uint32) y + 2147483648;
    __syscall ("additive3pp::max_uint32_vec", __domainid (D), in1, in2, in1);
    in1 -= 2147483648;
    return (int32)in1;
}
template <domain D : additive3pp>
D int max (D int x, D int y) {
    D uint in1 = (uint) x + 9223372036854775808;
    D uint in2 = (uint) y + 9223372036854775808;
    __syscall ("additive3pp::max_uint64_vec", __domainid (D), in1, in2, in1);
    in1 -= 9223372036854775808;
    return (int)in1;
}
template <domain D : additive3pp>
D uint8[[1]] max (D uint8[[1]] x, D uint8[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::max_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint16[[1]] max (D uint16[[1]] x, D uint16[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::max_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint32[[1]] max (D uint32[[1]] x, D uint32[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::max_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D uint[[1]] max (D uint[[1]] x, D uint[[1]] y) {
    assert (size (x) == size (y));
    __syscall ("additive3pp::max_uint64_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp>
D int8[[1]] max (D int8[[1]] x, D int8[[1]] y) {
    assert (size (x) == size (y));
    D uint8[[1]] in1 = (uint8) x + 128;
    D uint8[[1]] in2 = (uint8) y + 128;
    __syscall ("additive3pp::max_uint8_vec", __domainid (D), in1, in2, in1);
    in1 -= 128;
    return (int8)in1;
}
template <domain D : additive3pp>
D int16[[1]] max (D int16[[1]] x, D int16[[1]] y) {
    assert (size (x) == size (y));
    D uint16[[1]] in1 = (uint16) x + 32768;
    D uint16[[1]] in2 = (uint16) y + 32768;
    __syscall ("additive3pp::max_uint16_vec", __domainid (D), in1, in2, in1);
    in1 -= 32768;
    return (int16)in1;
}
template <domain D : additive3pp>
D int32[[1]] max (D int32[[1]] x, D int32[[1]] y) {
    assert (size (x) == size (y));
    D uint32[[1]] in1 = (uint32) x + 2147483648;
    D uint32[[1]] in2 = (uint32) y + 2147483648;
    __syscall ("additive3pp::max_uint32_vec", __domainid (D), in1, in2, in1);
    in1 -= 2147483648;
    return (int32)in1;
}
template <domain D : additive3pp>
D int[[1]] max (D int[[1]] x, D int[[1]] y) {
    assert (size (x) == size (y));
    D uint[[1]] in1 = (uint) x + 9223372036854775808;
    D uint[[1]] in2 = (uint) y + 9223372036854775808;
    __syscall ("additive3pp::max_uint64_vec", __domainid (D), in1, in2, in1);
    in1 -= 9223372036854775808;
    return (int)in1;
}
template <domain D : additive3pp, dim N>
D uint8[[N]] max (D uint8[[N]] x, D uint8[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::max_uint8_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D uint16[[N]] max (D uint16[[N]] x, D uint16[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::max_uint16_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D uint32[[N]] max (D uint32[[N]] x, D uint32[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::max_uint32_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D uint[[N]] max (D uint[[N]] x, D uint[[N]] y) {
    assert(shapesAreEqual(x,y));
    __syscall ("additive3pp::max_uint64_vec", __domainid (D), x, y, x);
    return x;
}
template <domain D : additive3pp, dim N>
D int8[[N]] max (D int8[[N]] x, D int8[[N]] y) {
    assert(shapesAreEqual(x,y));
    D uint8[[N]] in1 = (uint8) x + 128;
    D uint8[[N]] in2 = (uint8) y + 128;
    __syscall ("additive3pp::max_uint8_vec", __domainid (D), in1, in2, in1);
    in1 -= 128;
    return (int8)in1;
}
template <domain D : additive3pp, dim N>
D int16[[N]] max (D int16[[N]] x, D int16[[N]] y) {
    assert(shapesAreEqual(x,y));
    D uint16[[N]] in1 = (uint16) x + 32768;
    D uint16[[N]] in2 = (uint16) y + 32768;
    __syscall ("additive3pp::max_uint16_vec", __domainid (D), in1, in2, in1);
    in1 -= 32768;
    return (int16)in1;
}
template <domain D : additive3pp, dim N>
D int32[[N]] max (D int32[[N]] x, D int32[[N]] y) {
    assert(shapesAreEqual(x,y));
    D uint32[[N]] in1 = (uint32) x + 2147483648;
    D uint32[[N]] in2 = (uint32) y + 2147483648;
    __syscall ("additive3pp::max_uint32_vec", __domainid (D), in1, in2, in1);
    in1 -= 2147483648;
    return (int32)in1;
}
template <domain D : additive3pp, dim N>
D int[[N]] max (D int[[N]] x, D int[[N]] y) {
    assert(shapesAreEqual(x,y));
    D uint[[N]] in1 = (uint) x + 9223372036854775808;
    D uint[[N]] in2 = (uint) y + 9223372036854775808;
    __syscall ("additive3pp::max_uint64_vec", __domainid (D), in1, in2, in1);
    in1 -= 9223372036854775808;
    return (int)in1;
}

/** @}*/
/** @}*/
/** \addtogroup <a3p_floor>
 *  @{
 *  @brief Functions for rounding a value downwards
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float64 "float64"
 *  @return returns a \ref uint64 "uint" type value with the downwards rounded value of the input scalar/vector
 */

/**
* @param value - input scalar of supported type
*/
template <domain D : additive3pp>
D int64 floor (D float64 value) {
    D int64 out;
    __syscall("additive3pp::floor_float64_vec", __domainid( D ), value, out);
    return out;
}

/**
* @param arr - input vector of supported type
*/
template <domain D : additive3pp>
D int64[[1]] floor (D float64[[1]] arr) {
    D int64[[1]] out (size (arr));
    __syscall("additive3pp::floor_float64_vec", __domainid( D ), arr, out);
    return out;
}

/** @}*/
/** \addtogroup <a3p_ceiling>
 *  @{
 *  @brief Functions for rounding a value upwards
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float64 "float64"
 *  @return returns a \ref uint64 "uint" type value with the upwards rounded value of the input scalar/vector
 */

/**
* @param value - input scalar of supported type
*/
template <domain D : additive3pp>
D int64 ceiling (D float64 value) {
    D int64 out;
    __syscall("additive3pp::ceiling_float64_vec", __domainid( D ), value, out);
    return out;
}

/**
* @param arr - input vector of supported type
*/
template <domain D : additive3pp>
D int64[[1]] ceiling (D float64[[1]] arr) {
    D int64[[1]] out (size (arr));
    __syscall("additive3pp::ceiling_float64_vec", __domainid( D ), arr, out);
    return out;
}

/** @}*/
/** @}*/

















/**
 * \cond
 * Functions for accessing the main arguments.
 */

template <domain D : additive3pp, type T>
D T argument (string name) {
    uint num_bytes;
    __syscall ("process_get_argument", __cref name, __return num_bytes);
    uint8 [[1]] bytes (num_bytes);
    D T out;
    __syscall ("process_get_argument", __cref name, __ref bytes, __return num_bytes);
    __syscall ("additive3pp::set_shares_$T\_vec", __domainid(D), out, __cref bytes);
    return out;
}

template <domain D : additive3pp, type T>
D T[[1]] argument (string name) {
    uint num_bytes, vector_size;
    __syscall ("process_get_argument", __cref name, __return num_bytes);
    uint8 [[1]] bytes (num_bytes);
    __syscall ("process_get_argument", __cref name, __ref bytes, __return num_bytes);
    __syscall ("additive3pp::set_shares_$T\_vec", __domainid(D), __cref bytes, __return vector_size);
    D T [[1]] out (vector_size);
    __syscall ("additive3pp::set_shares_$T\_vec", __domainid(D), out, __cref bytes);
    return out;
}


/*
 * Functions for publishing values.
 */

template <domain D : additive3pp, type T, dim N>
void publish (string name, D T[[N]] val) {
    uint num_bytes;
    __syscall ("additive3pp::get_shares_$T\_vec", __domainid(D), val, __return num_bytes);
    uint8 [[1]] bytes (num_bytes);
    __syscall ("additive3pp::get_shares_$T\_vec", __domainid(D), val, __ref bytes);
    __syscall ("process_set_result", __cref name, __cref bytes);
}

/**
* \endcond
*/
