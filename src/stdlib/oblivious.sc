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
module oblivious;

import stdlib;
/**
* \endcond
*/

/**
* @file
* \defgroup oblivious oblivious.sc
* \defgroup choose choose
* \defgroup choose1 choose(single condition)
* \defgroup choose2 choose(multiple conditions)
* \defgroup vectorlookup vectorLookup
* \defgroup matrixlookuprow matrixLookupRow
* \defgroup matrixlookupcolumn matrixLookupColumn
* \defgroup matrixlookup matrixLookup
* \defgroup vectorupdate vectorUpdate
* \defgroup matrixupdaterow matrixUpdateRow
* \defgroup matrixupdatecolumn matrixUpdateColumn
* \defgroup matrixupdate matrixUpdate
*/

/** \addtogroup <oblivious> 
*@{
* @brief Module with functions for oblivious tasks
*/


/** 
 * \todo
 *    NB!!
 * Functions for float32 and float64 do not guarantee precise results!!!
 * Need syscalls for floats that compare all 3 components of float. *
 *
 */


/*******************************************************************************
********************************************************************************
**                                                                            **
**  Oblivious choice (choose)                                                 **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <choose> 
 *  @{
 *  @brief Function for obliviously choosing one of the inputs
 *  @note **D** - all protection domains
 *  @return returns one of the input arrays that was obliviously chosen with the condition
 */

/** \addtogroup <choose1> 
 *  @{
 *  @brief Function for obliviously choosing one of the inputs
 *  @note **D** - all protection domains
 *  @note **T** - any \ref data_types "data" type
 *  @param cond - a boolean scalar.
 *  @return returns one of the input arrays that was obliviously chosen with the condition. if **true**, array **first** is returned else **second** is returned 
 */

template <domain D, type T, dim N>
D T[[N]] choose(bool cond, D T[[N]] first, D T[[N]] second) {
    return cond ? first : second;
}
template <domain D, dim N>
D bool[[N]] choose(D bool cond, D bool[[N]] first, D bool[[N]] second) {
    assert(shapesAreEqual(first,second));
    return first && cond || second && !cond;
}
template <domain D, dim N>
D uint8[[N]] choose(D bool cond, D uint8[[N]] first, D uint8[[N]] second) {
    assert(shapesAreEqual(first,second));
    D uint8[[N]] cond2 = first;
    cond2 = (uint8)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D uint16[[N]] choose(D bool cond, D uint16[[N]] first, D uint16[[N]] second) {
    assert(shapesAreEqual(first,second));
    D uint16[[N]] cond2 = first;
    cond2 = (uint16)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D uint32[[N]] choose(D bool cond, D uint32[[N]] first, D uint32[[N]] second) {
    assert(shapesAreEqual(first,second));
    D uint32[[N]] cond2 = first;
    cond2 = (uint32)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D uint[[N]] choose(D bool cond, D uint[[N]] first, D uint[[N]] second) {
    assert(shapesAreEqual(first,second));
    D uint[[N]] cond2 = first;
    cond2 = (uint)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D int8[[N]] choose(D bool cond, D int8[[N]] first, D int8[[N]] second) {
    assert(shapesAreEqual(first,second));
    D int8[[N]] cond2 = first;
    cond2 = (int8)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D int16[[N]] choose(D bool cond, D int16[[N]] first, D int16[[N]] second) {
    assert(shapesAreEqual(first,second));
    D int16[[N]] cond2 = first;
    cond2 = (int16)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D int32[[N]] choose(D bool cond, D int32[[N]] first, D int32[[N]] second) {
    assert(shapesAreEqual(first,second));
    D int32[[N]] cond2 = first;
    cond2 = (int32)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D int[[N]] choose(D bool cond, D int[[N]] first, D int[[N]] second) {
    assert(shapesAreEqual(first,second));
    D int[[N]] cond2 = first;
    cond2 = (int)cond;
    return first * cond2 + second * (1 - cond2);
}

template <domain D, dim N>
D float32[[N]] choose(D bool cond, D float32[[N]] first, D float32[[N]] second) {
    assert(shapesAreEqual(first,second));
    D float32[[N]] cond2 = first;
    cond2 = (float32)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D float64[[N]] choose(D bool cond, D float64[[N]] first, D float64[[N]] second) {
    assert(shapesAreEqual(first,second));
    D float64[[N]] cond2 = first;
    cond2 = (float64)cond;
    return first * cond2 + second * (1 - cond2);
}

/** @}*/

/** \addtogroup <choose2> 
 *  @{
 *  @brief Function for obliviously choosing pointwise from the inputs
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param cond - a boolean array
 *  @return pointwise check if **cond** at a certain position is **true** or **false**. if **true** the element of **first** at that position is returned else the element of **second** at that position is returned
 */

template <domain D, dim N>
D bool[[N]] choose(D bool[[N]] cond, D bool[[N]] first, D bool[[N]] second) {
    uint[[1]] sFirst = shape(first);
    assert(all(sFirst == shape(cond) && all(sFirst == shape(second))));
    return first && cond || second && !cond;
}
template <domain D, dim N>
D uint8[[N]] choose(D bool[[N]] cond, D uint8[[N]] first, D uint8[[N]] second) {
    uint[[1]] sFirst = shape(first);
    assert(all(sFirst == shape(cond) && all(sFirst == shape(second))));
    D uint8[[N]] cond2 = (uint8)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D uint16[[N]] choose(D bool[[N]] cond, D uint16[[N]] first, D uint16[[N]] second) {
    uint[[1]] sFirst = shape(first);
    assert(all(sFirst == shape(cond) && all(sFirst == shape(second))));
    D uint16[[N]] cond2 = (uint16)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D uint32[[N]] choose(D bool[[N]] cond, D uint32[[N]] first, D uint32[[N]] second) {
    uint[[1]] sFirst = shape(first);
    assert(all(sFirst == shape(cond) && all(sFirst == shape(second))));
    D uint32[[N]] cond2 = (uint32)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D uint[[N]] choose(D bool[[N]] cond, D uint[[N]] first, D uint[[N]] second) {
    uint[[1]] sFirst = shape(first);
    assert(all(sFirst == shape(cond) && all(sFirst == shape(second))));
    D uint[[N]] cond2 = (uint)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D int8[[N]] choose(D bool[[N]] cond, D int8[[N]] first, D int8[[N]] second) {
    uint[[1]] sFirst = shape(first);
    assert(all(sFirst == shape(cond) && all(sFirst == shape(second))));
    D int8[[N]] cond2 = (int8)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D int16[[N]] choose(D bool[[N]] cond, D int16[[N]] first, D int16[[N]] second) {
    uint[[1]] sFirst = shape(first);
    assert(all(sFirst == shape(cond) && all(sFirst == shape(second))));
    D int16[[N]] cond2 = (int16)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D int32[[N]] choose(D bool[[N]] cond, D int32[[N]] first, D int32[[N]] second) {
    uint[[1]] sFirst = shape(first);
    assert(all(sFirst == shape(cond) && all(sFirst == shape(second))));
    D int32[[N]] cond2 = (int32)cond;
    return first * cond2 + second * (1 - cond2);
}
template <domain D, dim N>
D int[[N]] choose(D bool[[N]] cond, D int[[N]] first, D int[[N]] second) {
    uint[[1]] sFirst = shape(first);
    assert(all(sFirst == shape(cond) && all(sFirst == shape(second))));
    D int[[N]] cond2 = (int)cond;
    return first * cond2 + second * (1 - cond2);
}


/** @}*/
/** @}*/


/*******************************************************************************
********************************************************************************
**                                                                            **
**  vectorLookup                                                             **
**                                                                            **
********************************************************************************
*******************************************************************************/
/**
* \cond
*/
template <domain D>
D bool[[1]] vectorLookupBitmask(uint elems, D uint index) {
    // assert(declassify(elems > index));
    uint[[1]] is(elems);
    D uint[[1]] mask(elems);
    for (uint i = 0; i < elems; ++i) {
        is[i] = i;
        mask[i] = index;
    }
    return (mask == is);
}

/**
* \endcond
*/

/** \addtogroup <vectorlookup> 
 *  @{
 *  @brief Function for obliviously looking up an element in a vector
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param vec - a 1-dimensional vector of supported type
 *  @param index - an \ref uint64 "uint" type scalar for specifying the position in the vector to look up
 *  @return returns the element in the vector specified by **index**
 */

template <domain D>
D bool vectorLookup(D bool[[1]] vec, D uint index) {
    uint elems = size(vec);
    D bool[[1]] mask = vectorLookupBitmask(elems, index);
    mask &= vec;
    D bool sum;
    for (uint i = 0; i < elems; ++i) {
        sum ^= mask[i];
    }
    return sum;
}

template <domain D>
D uint8 vectorLookup(D uint8[[1]] vec, D uint index) {
    uint elems = size(vec);
    D uint8[[1]] mask = (uint8) vectorLookupBitmask(elems, index);
    mask *= vec;
    D uint8 sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

template <domain D>
D uint16 vectorLookup(D uint16[[1]] vec, D uint index) {
    uint elems = size(vec);
    D uint16[[1]] mask = (uint16) vectorLookupBitmask(elems, index);
    mask *= vec;
    D uint16 sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

template <domain D>
D uint32 vectorLookup(D uint32[[1]] vec, D uint index) {
    uint elems = size(vec);
    D uint32[[1]] mask = (uint32) vectorLookupBitmask(elems, index);
    mask *= vec;
    D uint32 sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

template <domain D>
D uint vectorLookup(D uint[[1]] vec, D uint index) {
    uint elems = size(vec);
    D uint[[1]] mask = (uint) vectorLookupBitmask(elems, index);
    mask *= vec;
    D uint sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

template <domain D>
D int8 vectorLookup(D int8[[1]] vec, D uint index) {
    uint elems = size(vec);
    D int8[[1]] mask = (int8) vectorLookupBitmask(elems, index);
    mask *= vec;
    D int8 sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

template <domain D>
D int16 vectorLookup(D int16[[1]] vec, D uint index) {
    uint elems = size(vec);
    D int16[[1]] mask = (int16) vectorLookupBitmask(elems, index);
    mask *= vec;
    D int16 sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

template <domain D>
D int32 vectorLookup(D int32[[1]] vec, D uint index) {
    uint elems = size(vec);
    D int32[[1]] mask = (int32) vectorLookupBitmask(elems, index);
    mask *= vec;
    D int32 sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

template <domain D>
D int vectorLookup(D int[[1]] vec, D uint index) {
    uint elems = size(vec);
    D int[[1]] mask = (int) vectorLookupBitmask(elems, index);
    mask *= vec;
    D int sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

template <domain D>
D float32 vectorLookup(D float32[[1]] vec, D uint index) {
    uint elems = size(vec);
    D float32[[1]] mask = (float32) vectorLookupBitmask(elems, index);
    mask *= vec;
    D float32 sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

template <domain D>
D float64 vectorLookup(D float64[[1]] vec, D uint index) {
    uint elems = size(vec);
    D float64[[1]] mask = (float64) vectorLookupBitmask(elems, index);
    mask *= vec;
    D float64 sum;
    for (uint i = 0; i < elems; ++i) {
        sum += mask[i];
    }
    return sum;
}

/** @}*/


/*******************************************************************************
********************************************************************************
**                                                                            **
**  matrixLookupRow                                                         **
**                                                                            **
********************************************************************************
*******************************************************************************/

/**
* \cond
*/

template <domain D>
D bool[[2]] matrixLookupRowBitmask(uint rows, uint cols, D uint rowIndex) {
    // assert(declassify(rows > rowIndex));
    D uint[[2]] mask(rows, 1);
    uint[[2]] is(rows, 1);
    for (uint i = 0; i < rows; ++i) {
        is[i, 0] = i;
        mask[i, 0] = rowIndex;
    }

    D bool[[2]] bitmask = (mask == is);

    // Stretch mask:
    for (uint i = 1; i < cols; ++i)
        bitmask = cat(bitmask, bitmask[:, :1], 1);

    return bitmask;
}

/**
* \endcond
*/

/** \addtogroup <matrixlookuprow> 
 *  @{
 *  @brief Function for obliviously looking up a row in a matrix
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a 2-dimensional matrix of supported type
 *  @param rowIndex - an \ref uint64 "uint" type scalar for specifying the row of the input matrix to look up
 *  @return returns the row from the input matrix specified by **rowIndex**
 */


template <domain D>
D bool[[1]] matrixLookupRow(D bool[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    mat &= mask;

    D bool[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i) {
        sum ^= mat[i, :];
    }

    return sum;
}

template <domain D>
D uint8[[1]] matrixLookupRow(D uint8[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D uint8[[2]] mask = (uint8) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D uint8[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

template <domain D>
D uint16[[1]] matrixLookupRow(D uint16[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D uint16[[2]] mask = (uint16) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D uint16[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

template <domain D>
D uint32[[1]] matrixLookupRow(D uint32[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D uint32[[2]] mask = (uint32) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D uint32[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

template <domain D>
D uint[[1]] matrixLookupRow(D uint[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D uint[[2]] mask = (uint) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D uint[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

template <domain D>
D int8[[1]] matrixLookupRow(D int8[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D int8[[2]] mask = (int8) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D int8[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

template <domain D>
D int16[[1]] matrixLookupRow(D int16[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D int16[[2]] mask = (int16) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D int16[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

template <domain D>
D int32[[1]] matrixLookupRow(D int32[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D int32[[2]] mask = (int32) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D int32[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

template <domain D>
D int[[1]] matrixLookupRow(D int[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D int[[2]] mask = (int) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D int[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

template <domain D>
D float32[[1]] matrixLookupRow(D float32[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D float32[[2]] mask = (float32) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D float32[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

template <domain D>
D float64[[1]] matrixLookupRow(D float64[[2]] mat, D uint rowIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D float64[[2]] mask = (float64) matrixLookupRowBitmask(rows, cols, rowIndex);

    mat *= mask;

    D float64[[1]] sum(cols);
    for (uint i = 0; i < rows; ++i)
        sum += mat[i, :];

    return sum;
}

/** @}*/

/*******************************************************************************
********************************************************************************
**                                                                            **
**  matrixLookupColumn                                                        **
**                                                                            **
********************************************************************************
*******************************************************************************/

/**
* \cond
*/

template <domain D>
D bool[[2]] matrixLookupColumnBitmask(uint rows, uint cols, D uint colIndex) {
    // assert(declassify(cols > colIndex));
    D uint[[2]] mask(1, cols);
    uint[[2]] is(1, cols);
    for (uint i = 0; i < cols; ++i) {
        is[0, i] = i;
        mask[0, i] = colIndex;
    }

    D bool[[2]] bitmask = (mask == is);

    // Stretch mask:
    for (uint i = 1; i < rows; ++i)
        bitmask = cat(bitmask, bitmask[:1, :], 0);

    return bitmask;
}
/**
* \endcond
*/

/** \addtogroup <matrixlookupcolumn> 
 *  @{
 *  @brief Function for obliviously looking up a column in a matrix
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a 2-dimensional matrix of supported type
 *  @param colIndex - an \ref uint64 "uint" type scalar for specifying the column of the input matrix to look up
 *  @return returns the column from the input matrix specified by **colIndex**
 */

template <domain D>
D bool[[1]] matrixLookupColumn (D bool[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    mat &= mask;

    D bool[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] ^= mat[i, j];

    return sum;
}

template <domain D>
D uint8[[1]] matrixLookupColumn(D uint8[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D uint8[[2]] mask = (uint8) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D uint8[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

template <domain D>
D uint16[[1]] matrixLookupColumn(D uint16[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D uint16[[2]] mask = (uint16) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D uint16[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

template <domain D>
D uint32[[1]] matrixLookupColumn(D uint32[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D uint32[[2]] mask = (uint32) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D uint32[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

template <domain D>
D uint[[1]] matrixLookupColumn(D uint[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D uint[[2]] mask = (uint) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D uint[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

template <domain D>
D int8[[1]] matrixLookupColumn(D int8[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D int8[[2]] mask = (int8) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D int8[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

template <domain D>
D int16[[1]] matrixLookupColumn(D int16[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D int16[[2]] mask = (int16) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D int16[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

template <domain D>
D int32[[1]] matrixLookupColumn(D int32[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D int32[[2]] mask = (int32) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D int32[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

template <domain D>
D int[[1]] matrixLookupColumn(D int[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D int[[2]] mask = (int) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D int[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

template <domain D>
D float32[[1]] matrixLookupColumn(D float32[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D float32[[2]] mask = (float32) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D float32[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

template <domain D>
D float64[[1]] matrixLookupColumn(D float64[[2]] mat, D uint colIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    D float64[[2]] mask = (float64) matrixLookupColumnBitmask(rows, cols, colIndex);

    mat *= mask;

    D float64[[1]] sum(rows);
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum[i] += mat[i, j];

    return sum;
}

/** @}*/

/*******************************************************************************
********************************************************************************
**                                                                            **
**  matrixLookup                                                             **
**                                                                            **
********************************************************************************
*******************************************************************************/

/**
* \cond
*/

template <domain D>
D bool[[2]] matrixLookupBitmask(uint rows, uint cols, D uint rowIndex, D uint columnIndex) {
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));
    // assert(declassify(cols > columnIndex));
    D uint[[2]] mask(rows, cols);
    uint[[2]] is(rows, cols);
    if (UINT64_MAX / rows < cols) { // if (rows * cols) would overflow
        D uint[[2]] mask2(rows, cols);
        uint[[2]] is2(rows, cols);
        for (uint i = 0; i < rows; ++i) {
            for (uint j = 0; j < cols; ++j) {
                is[i, j] = i;
                is2[i, j] = j;
                mask[i, j] = rowIndex;
                mask2[i, j] = columnIndex;
            }
        }
        return (mask == is) & (mask2 == is2);
    } else {
        D uint pIndex = rowIndex * cols + columnIndex;
        for (uint i = 0; i < rows; ++i) {
            for (uint j = 0; j < cols; ++j) {
                is[i, j] = i * cols + j;
                mask[i, j] = pIndex;
            }
        }
        return (mask == is);
    }
}

/**
* \endcond
*/

/** \addtogroup <matrixlookup> 
 *  @{
 *  @brief Function for obliviously looking up an element in the input matrix
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a 2-dimensional matrix of supported type
 *  @param rowIndex - an \ref uint64 "uint" type scalar for specifying the row in the input matrix
 *  @param colIndex - an \ref uint64 "uint" type scalar for specifying the column in the input matrix
 *  @return returns the element from the input matrix specified by **rowIndex** and **colIndex**  
 */

template <domain D>
D bool matrixLookup(D bool[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D bool[[2]] mask = matrixLookupBitmask(rows, cols, rowIndex, columnIndex);

    mat &= mask;

    D bool sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum ^= mat[i,j];

    return sum;
}

template <domain D>
D uint8 matrixLookup(D uint8[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D uint8[[2]] mask = (uint8) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D uint8 sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

template <domain D>
D uint16 matrixLookup(D uint16[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D uint16[[2]] mask = (uint16) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D uint16 sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

template <domain D>
D uint32 matrixLookup(D uint32[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D uint32[[2]] mask = (uint32) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D uint32 sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

template <domain D>
D uint matrixLookup(D uint[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D uint[[2]] mask = (uint) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D uint sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

template <domain D>
D int8 matrixLookup(D int8[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D int8[[2]] mask = (int8) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D int8 sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

template <domain D>
D int16 matrixLookup(D int16[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D int16[[2]] mask = (int16) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D int16 sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

template <domain D>
D int32 matrixLookup(D int32[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D int32[[2]] mask = (int32) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D int32 sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

template <domain D>
D int matrixLookup(D int[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D int[[2]] mask = (int) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D int sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

template <domain D>
D float32 matrixLookup(D float32[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D float32[[2]] mask = (float32) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D float32 sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

template <domain D>
D float64 matrixLookup(D float64[[2]] mat, D uint rowIndex, D uint columnIndex) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];

    D float64[[2]] mask = (float64) matrixLookupBitmask(rows, cols, rowIndex, columnIndex);


    mat *= mask;

    D float64 sum;
    for (uint i = 0; i < rows; ++i)
        for (uint j = 0; j < cols; ++j)
            sum += mat[i,j];

    return sum;
}

/** @}*/


/*******************************************************************************
********************************************************************************
**                                                                            **
**  vectorUpdate                                                             **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <vectorupdate> 
 *  @{
 *  @brief Function for obliviously updating an element in the input vector
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int"
 *  @param vec - a 1-dimensional vector of supported type
 *  @param index - an \ref uint64 "uint" type scalar for specifying the element to replace
 *  @param newValue - a scalar value of the same type as the input vector
 *  @return returns a vector with the value at position **index** replaced by **newValue**
 */

template <domain D>
D bool[[1]] vectorUpdate(D bool[[1]] vec, D uint index, D bool newValue) {
    D bool[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

template <domain D>
D int8[[1]] vectorUpdate(D int8[[1]] vec, D uint index, D int8 newValue) {
    D int8[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

template <domain D>
D int16[[1]] vectorUpdate(D int16[[1]] vec, D uint index, D int16 newValue) {
    D int16[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

template <domain D>
D int32[[1]] vectorUpdate(D int32[[1]] vec, D uint index, D int32 newValue) {
    D int32[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

template <domain D>
D int64[[1]] vectorUpdate(D int64[[1]] vec, D uint index, D int64 newValue) {
    D int64[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

template <domain D>
D uint8[[1]] vectorUpdate(D uint8[[1]] vec, D uint index, D uint8 newValue) {
    D uint8[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

template <domain D>
D uint16[[1]] vectorUpdate(D uint16[[1]] vec, D uint index, D uint16 newValue) {
    D uint16[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

template <domain D>
D uint32[[1]] vectorUpdate(D uint32[[1]] vec, D uint index, D uint32 newValue) {
    D uint32[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

template <domain D>
D uint[[1]] vectorUpdate(D uint64[[1]] vec, D uint index, D uint64 newValue) {
    D uint[[1]] n(size(vec)) = newValue;
    return choose(vectorLookupBitmask(size(vec), index), n, vec);
}

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

/*******************************************************************************
********************************************************************************
**                                                                            **
**  matrixUpdateRow                                                         **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <matrixupdaterow> 
 *  @{
 *  @brief Function for obliviously updating a row in the input matrix
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a matrix of supported type
 *  @param rowIndex - an \ref uint64 "uint" type scalar for specifying the row to replace
 *  @param newRow - a vector with new values
 *  @return returns a matrix where the row at **rowIndex** has been replaced with **newRow**
 */

template <domain D : additive3pp>
D bool[[2]] matrixUpdateRow(D bool[[2]] mat, D uint rowIndex, D bool[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D bool[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

template <domain D : additive3pp>
D uint8[[2]] matrixUpdateRow(D uint8[[2]] mat, D uint rowIndex, D uint8[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D uint8[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

template <domain D : additive3pp>
D uint16[[2]] matrixUpdateRow(D uint16[[2]] mat, D uint rowIndex, D uint16[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D uint16[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

template <domain D : additive3pp>
D uint32[[2]] matrixUpdateRow(D uint32[[2]] mat, D uint rowIndex, D uint32[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D uint32[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

template <domain D : additive3pp>
D uint[[2]] matrixUpdateRow(D uint[[2]] mat, D uint rowIndex, D uint[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D uint[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

template <domain D : additive3pp>
D int8[[2]] matrixUpdateRow(D int8[[2]] mat, D uint rowIndex, D int8[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D int8[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

template <domain D : additive3pp>
D int16[[2]] matrixUpdateRow(D int16[[2]] mat, D uint rowIndex, D int16[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D int16[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

template <domain D : additive3pp>
D int32[[2]] matrixUpdateRow(D int32[[2]] mat, D uint rowIndex, D int32[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D int32[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

template <domain D : additive3pp>
D int[[2]] matrixUpdateRow(D int[[2]] mat, D uint rowIndex, D int[[1]] newRow) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(cols == size(newRow));
    // assert(declassify(rows > rowIndex));

    D bool[[2]] mask = matrixLookupRowBitmask(rows, cols, rowIndex);

    D int[[2]] newRows(rows, cols);
    for (uint i = 0; i < rows; ++i)
        newRows[i,:] = newRow;

    return choose(mask, newRows, mat);
}

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

/*******************************************************************************
********************************************************************************
**                                                                            **
**  matrixUpdateColumn                                                      **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <matrixupdatecolumn> 
 *  @{
 *  @brief Function for obliviously updating a column in the input matrix
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a matrix of supported type
 *  @param colIndex - an \ref uint64 "uint" type scalar for specifying the column to replace
 *  @param newCol - a vector with new values
 *  @return returns a matrix where the column at **colIndex** has been replaced with **newCol**
 */

template <domain D : additive3pp>
D bool[[2]] matrixUpdateColumn(D bool[[2]] mat, D uint colIndex, D bool[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D bool[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

template <domain D : additive3pp>
D uint8[[2]] matrixUpdateColumn(D uint8[[2]] mat, D uint colIndex, D uint8[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D uint8[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

template <domain D : additive3pp>
D uint16[[2]] matrixUpdateColumn(D uint16[[2]] mat, D uint colIndex, D uint16[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D uint16[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

template <domain D : additive3pp>
D uint32[[2]] matrixUpdateColumn(D uint32[[2]] mat, D uint colIndex, D uint32[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D uint32[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

template <domain D : additive3pp>
D uint[[2]] matrixUpdateColumn(D uint[[2]] mat, D uint colIndex, D uint[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D uint[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

template <domain D : additive3pp>
D int8[[2]] matrixUpdateColumn(D int8[[2]] mat, D uint colIndex, D int8[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D int8[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

template <domain D : additive3pp>
D int16[[2]] matrixUpdateColumn(D int16[[2]] mat, D uint colIndex, D int16[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D int16[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

template <domain D : additive3pp>
D int32[[2]] matrixUpdateColumn(D int32[[2]] mat, D uint colIndex, D int32[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D int32[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

template <domain D : additive3pp>
D int[[2]] matrixUpdateColumn(D int[[2]] mat, D uint colIndex, D int[[1]] newCol) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    assert(rows == size(newCol));
    // assert(declassify(cols > colIndex));

    D bool[[2]] mask = matrixLookupColumnBitmask(rows, cols, colIndex);

    D int[[2]] newCols(rows, cols);
    for (uint i = 0; i < cols; ++i)
        newCols[:,i] = newCol;

    return choose(mask, newCols, mat);
}

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

/*******************************************************************************
********************************************************************************
**                                                                            **
**  matrixUpdate                                                             **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <matrixupdate> 
 *  @{
 *  @brief Function for obliviously updating a value in the input matrix
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a matrix of supported type
 *  @param rowIndex - an \ref uint64 "uint" type scalar for specifying the row in the input matrix
 *  @param colIndex - an \ref uint64 "uint" type scalar for specifying the column in the input matrix
 *  @param newValue - a new scalar value
 *  @return returns a matrix where the element at row **rowIndex** and column **colIndex** has been replaced with **newValue** 
 */

template <domain D : additive3pp>
D bool[[2]] matrixUpdate(D bool[[2]] mat, D uint rowIndex, D uint columnIndex, D bool newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D bool[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

template <domain D : additive3pp>
D uint8[[2]] matrixUpdate(D uint8[[2]] mat, D uint rowIndex, D uint columnIndex, D uint8 newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D uint8[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

template <domain D : additive3pp>
D uint16[[2]] matrixUpdate(D uint16[[2]] mat, D uint rowIndex, D uint columnIndex, D uint16 newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D uint16[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

template <domain D : additive3pp>
D uint32[[2]] matrixUpdate(D uint32[[2]] mat, D uint rowIndex, D uint columnIndex, D uint32 newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D uint32[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

template <domain D : additive3pp>
D uint[[2]] matrixUpdate(D uint[[2]] mat, D uint rowIndex, D uint columnIndex, D uint newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D uint[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

template <domain D : additive3pp>
D int8[[2]] matrixUpdate(D int8[[2]] mat, D uint rowIndex, D uint columnIndex, D int8 newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D int8[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

template <domain D : additive3pp>
D int16[[2]] matrixUpdate(D int16[[2]] mat, D uint rowIndex, D uint columnIndex, D int16 newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D int16[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

template <domain D : additive3pp>
D int32[[2]] matrixUpdate(D int32[[2]] mat, D uint rowIndex, D uint columnIndex, D int32 newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D int32[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

template <domain D : additive3pp>
D int[[2]] matrixUpdate(D int[[2]] mat, D uint rowIndex, D uint columnIndex, D int newValue) {
    uint[[1]] s = shape(mat);
    uint rows = s[0];
    uint cols = s[1];
    // assert(declassify(rows > rowIndex));
    // assert(declassify(cols > columnIndex));

    D int[[2]] n(rows, cols) = newValue;
    return choose(matrixLookupBitmask(rows, cols, rowIndex, columnIndex), n, mat);
}

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