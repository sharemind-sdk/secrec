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

import oblivious;
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


/** \addtogroup <vectorupdate>
 *  @{
 *  @brief Function for obliviously updating an element in the input vector
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float64 "float64" / \ref float32 "float32"
 *  @param vec - a 1-dimensional vector of supported type
 *  @param index - an \ref uint64 "uint" type scalar for specifying the element to replace
 *  @param newValue - a scalar value of the same type as the input vector
 *  @return returns a vector with the value at position **index** replaced by **newValue**
 */
template <domain D>
D float32[[1]] vectorUpdate(D float32[[1]] vec, D uint index, D float32 newValue) {
    D float32[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

template <domain D>
D float64[[1]] vectorUpdate(D float64[[1]] vec, D uint index, D float64 newValue) {
    D float64[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}
/** @}*/


/** \addtogroup <matrixupdaterow>
 *  @{
 *  @brief Function for obliviously updating a row in the input matrix
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a matrix of supported type
 *  @param rowIndex - an \ref uint64 "uint" type scalar for specifying the row to replace
 *  @param newRow - a vector with new values
 *  @return returns a matrix where the row at **rowIndex** has been replaced with **newRow**
 */

template <domain D : additive3pp>
D float32[[2]] matrixUpdateRow(D float32[[2]] mat, D uint rowIndex, D float32[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D float32[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

template <domain D : additive3pp>
D float64[[2]] matrixUpdateRow(D float64[[2]] mat, D uint rowIndex, D float64[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D float64[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}
/** @}*/


/** \addtogroup <matrixupdatecolumn>
 *  @{
 *  @brief Function for obliviously updating a column in the input matrix
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a matrix of supported type
 *  @param colIndex - an \ref uint64 "uint" type scalar for specifying the column to replace
 *  @param newCol - a vector with new values
 *  @return returns a matrix where the column at **colIndex** has been replaced with **newCol**
 */
template <domain D : additive3pp>
D float32[[2]] matrixUpdateColumn(D float32[[2]] mat, D uint colIndex, D float32[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D float32[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

template <domain D : additive3pp>
D float64[[2]] matrixUpdateColumn(D float64[[2]] mat, D uint colIndex, D float64[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D float64[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}
/** @}*/


/** \addtogroup <matrixupdate>
 *  @{
 *  @brief Function for obliviously updating a value in the input matrix
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a matrix of supported type
 *  @param rowIndex - an \ref uint64 "uint" type scalar for specifying the row in the input matrix
 *  @param colIndex - an \ref uint64 "uint" type scalar for specifying the column in the input matrix
 *  @param newValue - a new scalar value
 *  @return returns a matrix where the element at row **rowIndex** and column **colIndex** has been replaced with **newValue**
 */
template <domain D : additive3pp>
D float32[[2]] matrixUpdate(D float32[[2]] mat, D uint rowIndex, D uint columnIndex, D float32 newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D float32[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

template <domain D : additive3pp>
D float64[[2]] matrixUpdate(D float64[[2]] mat, D uint rowIndex, D uint columnIndex, D float64 newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D float64[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}
/** @}*/

/** @}*/
