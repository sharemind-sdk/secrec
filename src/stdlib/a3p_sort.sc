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
module a3p_sort;

import stdlib;
import additive3pp;
import a3p_random;
import oblivious;
import a3p_oblivious;
import xor3pp;
/**
* \endcond
*/

/**
* @file
* \defgroup a3p_sort a3p_sort.sc
* \defgroup sort sort
* \defgroup sort_vec sort[[1]]
* \defgroup sort_mat sort[[2]]
* \defgroup sortingnetwork sortingNetworkSort
* \defgroup sortingnetwork_vec sortingNetworkSort[[1]]
* \defgroup sortingnetwork_mat sortingNetworkSort[[2]](1 column)
* \defgroup sortingnetwork_mat2 sortingNetworkSort[[2]](2 columns)
* \defgroup sortingnetwork_mat3 sortingNetworkSort[[2]](3 columns)
* \defgroup selectk selectK
* \defgroup selectk_vec selectK[[1]]
* \defgroup selectk_mat selectK[[2]]
*/

/** \addtogroup <a3p_sort>
*@{
* @brief Module with functions for sorting values
*/

/*******************************************************************************
********************************************************************************
**                                                                            **
**  sort                                                                      **
**                                                                            **
********************************************************************************
*******************************************************************************/

/** \addtogroup <sort>
 *  @{
 *  @brief Functions for sorting values
 *  @note **D** - additive3pp protection domain
 *  @note **T** - any \ref data_types "data" type
 *  @note boolean values are sorted after their numerical value. **false** first then **true**
 */

/** \addtogroup <sort_vec>
 *  @{
 *  @brief Function for sorting values in a vector
 *  @note **D** - additive3pp protection domain
 *  @return returns a sorted vector from smaller to bigger values
 */


/**
* @note Supported types - \ref bool "bool"
*  @note boolean values are sorted after their numerical value. **false** first then **true**
* @param arr - a 1-dimensonal boolean vector
*/
template <domain D : additive3pp>
D bool[[1]] sort(D bool[[1]] arr) {
    D uint [[1]] vec = (uint) arr;
    D uint [[1]] ivec = 1 - vec;

    uint n = size (arr);

    D uint [[1]] pTrue (n);
    D uint acc = 0;
    for (uint i = 0; i < n; ++ i) {
        pTrue[i] = acc;
        acc += ivec[i];
    }

    D uint [[1]] pFalse (n);
    acc = n - 1;
    for (uint i = 1; i <= n; ++ i) {
        pFalse[n-i] = acc;
        acc -= vec[n-i];
    }

    // vec*pFalse + ivec*pTrue
    // ivec = 1-vec
    D uint[[1]] indexes = vec * (pFalse - pTrue) + pTrue;
    uint[[1]] publishedIndexes = declassify(indexes);
    D bool[[1]] sortedArr (n);
    for (uint i = 0; i < n; ++i) {
        sortedArr[publishedIndexes[i]] = arr[i];
    }

    return sortedArr;
}

/**
* @note **T** - any \ref data_types "data" type
* @param vec - a 1-dimensonal supported type vector
*/
template <domain D : additive3pp, type T>
D T[[1]] sort(D T[[1]] vec) {
    if (size(vec) <= 1)
        return vec;

    vec = shuffle(vec);

    uint n = size(vec);
    uint compSize = (n * (n - 1)) / 2;

    // Do the comparisons:
    /// \todo do check for matrix size so that comps etc can fit into memory
    D uint[[1]] comps;
    {
        // Generate the arrays to compare:
        D T[[1]] cArray1(compSize);
        D T[[1]] cArray2(compSize);
        uint i = 0;
        for (uint r = 0; r < n - 1; ++r) {
            for (uint c = r + 1; c < n; ++c) {
                cArray1[i] = vec[r];
                cArray2[i] = vec[c];
                ++i;
            }
        }

        // Do all the actual comparisons:
        comps = (uint) (cArray1 <= cArray2);
    }

    // Privately compute the new indexes:
    D uint[[1]] newIndexes(n);
    uint constOne = 1;
    {
        uint r = 0;
        uint c = 1;
        for (uint i = 0; i < compSize; ++i) {
            D uint v = comps[i];
            newIndexes[r] += constOne - v;
            newIndexes[c] += v;

            ++c;
            assert(c <= n);
            if (c >= n) {
                ++r;
                c = r + 1;
            }
        }
    }

    uint[[1]] publishedIndexes = declassify(newIndexes);

    D T[[1]] sorted(n);
    for (uint r = 0; r < n; ++r) {
        sorted[publishedIndexes[r]] = vec[r];
    }

    return sorted;
}
/** @}*/
/** \addtogroup <sort_mat>
 *  @{
 *  @brief Function for sorting columns in a matrix
 *  @note **D** - additive3pp protection domain
 *  @return returns a matrix with the specified column sorted from smaller to bigger values
 */

/**
 *  @note Supported types - \ref bool "bool"
 *  @note boolean values are sorted after their numerical value. **false** first then **true**
 *  @param column - an \ref uint64 "uint" type value for specifying the column to be sorted
 *  @param matrix - a matrix of supported type
*/
template <domain D : additive3pp>
D bool[[2]] sort(D bool[[2]] matrix, uint column) {
    uint[[1]] matShape = shape(matrix);

    D uint[[1]] sortCol = (uint) matrix[:, column];
    D uint[[1]] isortCol = 1 - sortCol;

    uint n = matShape[0];
    {
        D uint[[1]] pTrue (n);
        D uint acc = 0;
        for (uint i = 0; i < n; ++i) {
            pTrue[i] = acc;
            acc += isortCol[i];
        }

        isortCol *= pTrue;
    }

    {
        D uint[[1]] pFalse (n);
        D uint acc = n - 1;
        for (uint i = 1; i <= n; ++i) {
            pFalse[n-i] = acc;
            acc -= sortCol[n-i];
        }

        sortCol *= pFalse;
    }

    uint[[1]] publishedIndexes = declassify(sortCol + isortCol);
    D bool[[2]] sortedMatrix (matShape[0], matShape[1]);
    for (uint i = 0; i < n; ++i) {
        sortedMatrix[publishedIndexes[i], :] = matrix[i, :];
    }

    return sortedMatrix;
}

/**
 *  @note **T** - any \ref data_types "data" type
 *  @note boolean values are sorted after their numerical value. **false** first then **true**
 *  @param column - an \ref uint64 "uint" type value for specifying the column to be sorted
 *  @param matrix - a matrix of supported type
*/

template <domain D : additive3pp, type T>
D T[[2]] sort(D T[[2]] matrix, uint column) {
    uint n = shape(matrix)[0];
    if (n <= 1)
        return matrix;

    uint columnCount = shape(matrix)[1];
    assert(column < columnCount);
    matrix = shuffleRows(matrix);

    uint64 compSize = (n * (n - 1)) / 2;

    // Do the comparisons:
    /// \todo do check for matrix size so that comps etc can fit into memory
    D uint[[1]] comps;
    {
        // Generate the arrays to compare:
        D T[[1]] cArray1(compSize);
        D T[[1]] cArray2(compSize);
        uint i = 0;
        for (uint r = 0; r < n - 1; ++r) {
            for (uint c = r + 1; c < n; ++c) {
                cArray1[i] = matrix[r, column];
                cArray2[i] = matrix[c, column];
                ++i;
            }
        }

        // Do all the actual comparisons:
        comps = (uint) (cArray1 <= cArray2);
    }

    // Privately compute the new indexes:
    D uint[[1]] newIndexes(n);
    uint constOne = 1;
    {
        uint r = 0;
        uint c = 1;
        for (uint i = 0; i < compSize; ++i) {
            D uint v = comps[i];
            newIndexes[r] += constOne - v;
            newIndexes[c] += v;

            ++c;
            assert(c <= n);
            if (c >= n) {
                ++r;
                c = r + 1;
            }
        }
    }

    uint[[1]] publishedIndexes = declassify(newIndexes);

    D T[[2]] sorted(n, columnCount);
    for (uint r = 0; r < n; ++r)
        sorted[publishedIndexes[r], :] = matrix[r, :];

    return sorted;
}
/** @}*/
/** @}*/

/*******************************************************************************
********************************************************************************
**                                                                            **
**  sortingNetworkSort                                                        **
**                                                                            **
********************************************************************************
*******************************************************************************/

/**
* \cond
*/
uint[[1]] generateSortingNetwork(uint arraysize) {
    uint snsize = 0;
    __syscall("sorting_network_get_size", arraysize, __return snsize);
    uint[[1]] sn (snsize);
    __syscall("sorting_network_get", arraysize, __ref sn);
    return sn;
}
/**
* \endcond
*/


/** \addtogroup <sortingnetwork>
 *  @{
 *  @brief Functions for sorting values with sorting network
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 */

/** \addtogroup <sortingnetwork_vec>
 *  @{
 *  @brief Function for sorting values in a vector with sorting network
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @param array - a vector of supported type
 *  @return returns a sorted vector from smaller to bigger values
 */


//is this only additive3pp?
//doesn't work for bools
/**
* \cond
*/
template <domain D>
D uint8[[1]] sortingNetworkSort (D uint8[[1]] array) {
    D xor_uint8[[1]] sortIn = reshare (array);
    D xor_uint8[[1]] sortOut = sortingNetworkSort (sortIn);
    return reshare(sortOut);
}

template <domain D>
D uint16[[1]] sortingNetworkSort (D uint16[[1]] array) {
    D xor_uint16[[1]] sortIn = reshare (array);
    D xor_uint16[[1]] sortOut = sortingNetworkSort (sortIn);
    return reshare(sortOut);
}

template <domain D>
D uint32[[1]] sortingNetworkSort (D uint32[[1]] array) {
    D xor_uint32[[1]] sortIn = reshare (array);
    D xor_uint32[[1]] sortOut = sortingNetworkSort (sortIn);
    return reshare(sortOut);
}

template <domain D>
D uint64[[1]] sortingNetworkSort (D uint64[[1]] array) {
    D xor_uint64[[1]] sortIn = reshare (array);
    D xor_uint64[[1]] sortOut = sortingNetworkSort (sortIn);
    return reshare(sortOut);
}

template <domain D>
D int8[[1]] sortingNetworkSort (D int8[[1]] array) {
    D uint8[[1]] y = (uint8)array + 128;
    D xor_uint8[[1]] sortIn = reshare (y);
    D xor_uint8[[1]] sortOut = sortingNetworkSort (sortIn);
    y = reshare(sortOut) - 128;
    return (int8)y;
}

template <domain D>
D int16[[1]] sortingNetworkSort (D int16[[1]] array) {
    D uint16[[1]] y = (uint16)array + 32768;
    D xor_uint16[[1]] sortIn = reshare (y);
    D xor_uint16[[1]] sortOut = sortingNetworkSort (sortIn);
    y = reshare(sortOut) - 32768;
    return (int16)y;
}

template <domain D>
D int32[[1]] sortingNetworkSort (D int32[[1]] array) {
    D uint32[[1]] y = (uint32)array + 2147483648;
    D xor_uint32[[1]] sortIn = reshare (y);
    D xor_uint32[[1]] sortOut = sortingNetworkSort (sortIn);
    y = reshare(sortOut) - 2147483648;
    return (int32)y;
}

template <domain D>
D int64[[1]] sortingNetworkSort (D int64[[1]] array) {
    D uint64[[1]] y = (uint)array + 9223372036854775808;
    D xor_uint64[[1]] sortIn = reshare (y);
    D xor_uint64[[1]] sortOut = sortingNetworkSort (sortIn);
    y = reshare(sortOut) - 9223372036854775808;
    return (int64)y;
}
/**
* \endcond
*/
template <domain D, type T>
D T[[1]] sortingNetworkSort (D T[[1]] array) {

    // Generate sorting network
    uint[[1]] sortnet = generateSortingNetwork (size(array));

    // We will use this offset to decode the sorting network
    uint offset = 0;

    // Extract the number of stages
    uint numOfStages = sortnet[offset++];

    for (uint stage = 0; stage < numOfStages; stage++) {
        uint sizeOfStage = sortnet[offset++];

        D T[[1]] firstVector (sizeOfStage);
        D T[[1]] secondVector (sizeOfStage);
        D bool[[1]] exchangeFlagsVector (sizeOfStage);

        // Set up first comparison vector
        for (uint i = 0; i < sizeOfStage; ++i) {
            firstVector[i] = array[sortnet[offset]];
            offset++;
        }

        // Set up second comparison vector
        for (uint i = 0; i < sizeOfStage; ++i) {
            secondVector[i] = array[sortnet[offset]];
            offset++;
        }

        // Perform compares
        exchangeFlagsVector = firstVector <= secondVector;

        D bool[[1]] expandedExchangeFlagsVector (2 * sizeOfStage);

        uint counter = 0;
        for(uint i = 0; i < 2 * sizeOfStage; i = i + 2){
            expandedExchangeFlagsVector[i] =  exchangeFlagsVector[counter];
            expandedExchangeFlagsVector[i + 1] = exchangeFlagsVector[counter];
            counter++;
        }

        // Perform exchanges
        D T[[1]] firstFactor (2 * sizeOfStage);
        D T[[1]] secondFactor (2 * sizeOfStage);

        counter = 0;
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {

            firstFactor[i] = firstVector[counter];
            firstFactor[i + 1] = secondVector[counter];

            // Comparison bits

            secondFactor[i] = secondVector[counter];
            secondFactor[i + 1] = firstVector[counter];
            counter++;
        }

        D T[[1]] choiceResults (2 * sizeOfStage);

        choiceResults = choose(expandedExchangeFlagsVector,firstFactor,secondFactor);

        // Finalize oblivious choices
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {
            array[sortnet[offset++]] = choiceResults [i];
        }
        for (uint i = 1; i < (2 * sizeOfStage + 1); i = i + 2) {
            array[sortnet[offset++]] = choiceResults [i];
        }


    }
    return array;
}



/** @}*/
/** \addtogroup <sortingnetwork_mat>
 *  @{
 *  @brief Function for sorting a specified column in a matrix with sorting network
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @param column - an \ref uint64 "uint" type value specifying the column of the input matrix to be sorted
 *  @param matrix - a matrix of supported type
 *  @return returns a matrix with the specified column of the input matrix sorted from smaller to bigger values
 */
/**
* \cond
*/

template <domain D>
D uint8[[2]] sortingNetworkSort (D uint8[[2]] array, uint column) {
    D xor_uint8[[2]] sortIn = reshare (array);
    D xor_uint8[[2]] sortOut = sortingNetworkSort (sortIn,column);
    return reshare(sortOut);
}

template <domain D>
D uint16[[2]] sortingNetworkSort (D uint16[[2]] array, uint column) {
    D xor_uint16[[2]] sortIn = reshare (array);
    D xor_uint16[[2]] sortOut = sortingNetworkSort (sortIn,column);
    return reshare(sortOut);
}

template <domain D>
D uint32[[2]] sortingNetworkSort (D uint32[[2]] array, uint column) {
    D xor_uint32[[2]] sortIn = reshare (array);
    D xor_uint32[[2]] sortOut = sortingNetworkSort (sortIn,column);
    return reshare(sortOut);
}

template <domain D>
D uint64[[2]] sortingNetworkSort (D uint64[[2]] array, uint column) {
    D xor_uint64[[2]] sortIn = reshare (array);
    D xor_uint64[[2]] sortOut = sortingNetworkSort (sortIn,column);
    return reshare(sortOut);
}

template <domain D>
D int8[[2]] sortingNetworkSort (D int8[[2]] array, uint column) {
    D uint8[[2]] y = (uint8)array + 128;
    D xor_uint8[[2]] sortIn = reshare (y);
    D xor_uint8[[2]] sortOut = sortingNetworkSort (sortIn,column);
    y = reshare(sortOut) - 128;
    return (int8)y;
}

template <domain D>
D int16[[2]] sortingNetworkSort (D int16[[2]] array, uint column) {
    D uint16[[2]] y = (uint16)array + 32768;
    D xor_uint16[[2]] sortIn = reshare (y);
    D xor_uint16[[2]] sortOut = sortingNetworkSort (sortIn,column);
    y = reshare(sortOut) - 32768;
    return (int16)y;
}

template <domain D>
D int32[[2]] sortingNetworkSort (D int32[[2]] array, uint column) {
    D uint32[[2]] y = (uint32)array + 2147483648;
    D xor_uint32[[2]] sortIn = reshare (y);
    D xor_uint32[[2]] sortOut = sortingNetworkSort (sortIn,column);
    y = reshare(sortOut) - 2147483648;
    return (int32)y;
}

template <domain D>
D int64[[2]] sortingNetworkSort (D int64[[2]] array, uint column) {
    D uint64[[2]] y = (uint)array + 9223372036854775808;
    D xor_uint64[[2]] sortIn = reshare (y);
    D xor_uint64[[2]] sortOut = sortingNetworkSort (sortIn,column);
    y = reshare(sortOut) - 9223372036854775808;
    return (int64)y;
}
/**
* \endcond
*/
template <domain D, type T>
D T[[2]] sortingNetworkSort (D T[[2]] matrix, uint column) {
    uint[[1]] matShape = shape(matrix);

    // Generate sorting network
    uint[[1]] sortnet = generateSortingNetwork (matShape[0]);

    // We will use this offset to decode the sorting network
    uint offset = 0;

    // Extract the number of stages
    uint numOfStages = sortnet[offset++];

    for (uint stage = 0; stage < numOfStages; stage++) {
        uint sizeOfStage = sortnet[offset++];

        D T[[2]] firstVector (sizeOfStage, matShape[1]);
        D T[[2]] secondVector (sizeOfStage, matShape[1]);
        D bool[[1]] exchangeFlagsVector (sizeOfStage);

        // Set up first comparison vector
        for (uint i = 0; i < sizeOfStage; ++i) {
            firstVector[i, :] = matrix[sortnet[offset], :];
            offset++;
        }

        // Set up second comparison vector
        for (uint i = 0; i < sizeOfStage; ++i) {
            secondVector[i, :] = matrix[sortnet[offset], :];
            offset++;
        }

        // Perform compares
        exchangeFlagsVector = firstVector[:, column] <= secondVector[:, column];

        D bool[[2]] expandedExchangeFlagsVector (2 * sizeOfStage, matShape[1]);

        uint counter = 0;
        for(uint i = 0; i < 2 * sizeOfStage; i = i + 2){
            expandedExchangeFlagsVector[i,:] = exchangeFlagsVector[counter];
            expandedExchangeFlagsVector[i + 1,:] = exchangeFlagsVector[counter];
            counter++;
        }

        // Perform exchanges
        D T[[2]] firstFactor (2 * sizeOfStage, matShape[1]);
        D T[[2]] secondFactor (2 * sizeOfStage, matShape[1]);

        counter = 0;
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {

            firstFactor[i, :] = firstVector[counter, :];
            firstFactor[i + 1, :] = secondVector[counter, :];

            // Comparison bits

            secondFactor[i, :] = secondVector[counter, :];
            secondFactor[i + 1, :] = firstVector[counter, :];
            counter++;
        }

        // Run the largest multiplication this side of Dantoiine
        D T[[2]] choiceResults (2 * sizeOfStage, matShape[1]);

        choiceResults = choose(expandedExchangeFlagsVector,firstFactor,secondFactor);

        // Finalize oblivious choices
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {
            matrix[sortnet[offset++], :] = choiceResults [i, :];
        }

        for (uint i = 1; i < (2 * sizeOfStage + 1); i = i + 2) {
            matrix[sortnet[offset++], :] = choiceResults [i, :];
        }

    }
    return matrix;
}

/** @}*/
/** \addtogroup <sortingnetwork_mat2>
 *  @{
 *  @brief Function for sorting specified columns in a matrix with sorting network
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @param column1 - an \ref uint64 "uint" type value specifying the column of the input matrix to be sorted
 *  @param column2 - an \ref uint64 "uint" type value specifying the column of the input matrix to be sorted as a secondary sort parameter
 *  @param matrix - a matrix of supported type
 *  @return returns a matrix with the specified column of the input matrix sorted from smaller to bigger values
 */

/**
* \cond
*/

template <domain D>
D uint8[[2]] sortingNetworkSort (D uint8[[2]] array, uint column1, uint column2) {
    D xor_uint8[[2]] sortIn = reshare (array);
    D xor_uint8[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2);
    return reshare(sortOut);
}

template <domain D>
D uint16[[2]] sortingNetworkSort (D uint16[[2]] array, uint column1, uint column2) {
    D xor_uint16[[2]] sortIn = reshare (array);
    D xor_uint16[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2);
    return reshare(sortOut);
}

template <domain D>
D uint32[[2]] sortingNetworkSort (D uint32[[2]] array, uint column1, uint column2) {
    D xor_uint32[[2]] sortIn = reshare (array);
    D xor_uint32[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2);
    return reshare(sortOut);
}

template <domain D>
D uint64[[2]] sortingNetworkSort (D uint64[[2]] array, uint column1, uint column2) {
    D xor_uint64[[2]] sortIn = reshare (array);
    D xor_uint64[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2);
    return reshare(sortOut);
}

template <domain D>
D int8[[2]] sortingNetworkSort (D int8[[2]] array, uint column1, uint column2) {
    D uint8[[2]] y = (uint8)array + 128;
    D xor_uint8[[2]] sortIn = reshare (y);
    D xor_uint8[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2);
    y = reshare(sortOut) - 128;
    return (int8)y;
}

template <domain D>
D int16[[2]] sortingNetworkSort (D int16[[2]] array, uint column1, uint column2) {
    D uint16[[2]] y = (uint16)array + 32768;
    D xor_uint16[[2]] sortIn = reshare (y);
    D xor_uint16[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2);
    y = reshare(sortOut) - 32768;
    return (int16)y;
}

template <domain D>
D int32[[2]] sortingNetworkSort (D int32[[2]] array, uint column1, uint column2) {
    D uint32[[2]] y = (uint32)array + 2147483648;
    D xor_uint32[[2]] sortIn = reshare (y);
    D xor_uint32[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2);
    y = reshare(sortOut) - 2147483648;
    return (int32)y;
}

template <domain D>
D int64[[2]] sortingNetworkSort (D int64[[2]] array, uint column1, uint column2) {
    D uint64[[2]] y = (uint)array + 9223372036854775808;
    D xor_uint64[[2]] sortIn = reshare (y);
    D xor_uint64[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2);
    y = reshare(sortOut) - 9223372036854775808;
    return (int64)y;
}
/**
* \endcond
*/
template <domain D, type T>
D T[[2]] sortingNetworkSort (D T[[2]] matrix, uint column1 , uint column2) {
    uint[[1]] matShape = shape(matrix);

    // Generate sorting network
    uint[[1]] sortnet = generateSortingNetwork (matShape[0]);

    // We will use this offset to decode the sorting network
    uint offset = 0;

    // Extract the number of stages
    uint numOfStages = sortnet[offset++];

    for (uint stage = 0; stage < numOfStages; stage++) {
        uint sizeOfStage = sortnet[offset++];

        D T[[2]] firstVector (sizeOfStage, matShape[1]);
        D T[[2]] secondVector (sizeOfStage, matShape[1]);
        D bool[[1]] exchangeFlagsVector (sizeOfStage);

        // Set up first comparison vector
        for (uint i = 0; i < sizeOfStage; ++i) {
            firstVector[i, :] = matrix[sortnet[offset], :];
            offset++;
        }

        // Set up second comparison vector
        for (uint i = 0; i < sizeOfStage; ++i) {
            secondVector[i, :] = matrix[sortnet[offset], :];
            offset++;
        }

        // Perform compares in parallel as much as possible
        uint colSize = size (firstVector[:, column1]);
        D T[[1]] firstComparisonVector (2 * colSize);
        firstComparisonVector[0:colSize] = firstVector[:, column1];
        firstComparisonVector[colSize:2*colSize] = secondVector[:, column2];
        D T[[1]] secondComparisonVector (2 * colSize);
        secondComparisonVector[0:colSize] = secondVector[:, column1];
        secondComparisonVector[colSize:2*colSize] = firstVector[:, column2];
        D bool[[1]] greaterThanResult (2 * colSize);
        greaterThanResult = firstComparisonVector >= secondComparisonVector;

        // Perform compares
        exchangeFlagsVector = !(greaterThanResult[0:colSize]) || (firstVector[:, column1] == secondVector[:, column1] && greaterThanResult[colSize:2*colSize]);

        D bool[[2]] expandedExchangeFlagsVector (2 * sizeOfStage, matShape[1]);

        uint counter = 0;
        for(uint i = 0; i < 2 * sizeOfStage; i = i + 2){
            expandedExchangeFlagsVector[i,:] = exchangeFlagsVector[counter];
            expandedExchangeFlagsVector[i + 1,:] = exchangeFlagsVector[counter];
            counter++;
        }

        // Perform exchanges
        D T[[2]] firstFactor (2 * sizeOfStage, matShape[1]);
        D T[[2]] secondFactor (2 * sizeOfStage, matShape[1]);

        counter = 0;
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {

            firstFactor[i, :] = firstVector[counter, :];
            firstFactor[i + 1, :] = secondVector[counter, :];

            // Comparison bits

            secondFactor[i, :] = secondVector[counter, :];
            secondFactor[i + 1, :] = firstVector[counter, :];
            counter++;
        }

        // Run the largest multiplication this side of Dantoiine
        D T[[2]] choiceResults (2 * sizeOfStage, matShape[1]);

        choiceResults = choose(expandedExchangeFlagsVector,firstFactor,secondFactor);

        // Finalize oblivious choices
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {
            matrix[sortnet[offset++], :] = choiceResults [i, :];
        }

        for (uint i = 1; i < (2 * sizeOfStage + 1); i = i + 2) {
            matrix[sortnet[offset++], :] = choiceResults [i, :];
        }

    }
    return matrix;
}
/** @}*/
/** \addtogroup <sortingnetwork_mat3>
 *  @{
 *  @brief Function for sorting specified columns in a matrix with sorting network
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @param column1 - an \ref uint64 "uint" type value specifying the column of the input matrix to be sorted
 *  @param column2 - an \ref uint64 "uint" type value specifying the column of the input matrix to be sorted as a secondary sort parameter
 *  @param column3 - an \ref uint64 "uint" type value specifying the column of the input matrix to be sorted as a third sort parameter
 *  @param matrix - a matrix of supported type
 *  @return returns a matrix with the specified column of the input matrix sorted from smaller to bigger values
 */

/**
* \cond
*/

template <domain D>
D uint8[[2]] sortingNetworkSort (D uint8[[2]] array, uint column1, uint column2, uint column3) {
    D xor_uint8[[2]] sortIn = reshare (array);
    D xor_uint8[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2,column3);
    return reshare(sortOut);
}

template <domain D>
D uint16[[2]] sortingNetworkSort (D uint16[[2]] array, uint column1, uint column2, uint column3) {
    D xor_uint16[[2]] sortIn = reshare (array);
    D xor_uint16[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2,column3);
    return reshare(sortOut);
}

template <domain D>
D uint32[[2]] sortingNetworkSort (D uint32[[2]] array, uint column1, uint column2, uint column3) {
    D xor_uint32[[2]] sortIn = reshare (array);
    D xor_uint32[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2,column3);
    return reshare(sortOut);
}

template <domain D>
D uint64[[2]] sortingNetworkSort (D uint64[[2]] array, uint column1, uint column2, uint column3) {
    D xor_uint64[[2]] sortIn = reshare (array);
    D xor_uint64[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2,column3);
    return reshare(sortOut);
}

template <domain D>
D int8[[2]] sortingNetworkSort (D int8[[2]] array, uint column1, uint column2, uint column3) {
    D uint8[[2]] y = (uint8)array + 128;
    D xor_uint8[[2]] sortIn = reshare (y);
    D xor_uint8[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2,column3);
    y = reshare(sortOut) - 128;
    return (int8)y;
}

template <domain D>
D int16[[2]] sortingNetworkSort (D int16[[2]] array, uint column1, uint column2, uint column3) {
    D uint16[[2]] y = (uint16)array + 32768;
    D xor_uint16[[2]] sortIn = reshare (y);
    D xor_uint16[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2,column3);
    y = reshare(sortOut) - 32768;
    return (int16)y;
}

template <domain D>
D int32[[2]] sortingNetworkSort (D int32[[2]] array, uint column1, uint column2, uint column3) {
    D uint32[[2]] y = (uint32)array + 2147483648;
    D xor_uint32[[2]] sortIn = reshare (y);
    D xor_uint32[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2,column3);
    y = reshare(sortOut) - 2147483648;
    return (int32)y;
}

template <domain D>
D int64[[2]] sortingNetworkSort (D int64[[2]] array, uint column1, uint column2, uint column3) {
    D uint64[[2]] y = (uint)array + 9223372036854775808;
    D xor_uint64[[2]] sortIn = reshare (y);
    D xor_uint64[[2]] sortOut = sortingNetworkSort (sortIn,column1,column2,column3);
    y = reshare(sortOut) - 9223372036854775808;
    return (int64)y;
}
/**
* \endcond
*/
template <domain D, type T>
D T[[2]] sortingNetworkSort (D T[[2]] matrix, uint column1 , uint column2, uint column3) {
    uint[[1]] matShape = shape(matrix);

    // Generate sorting network
    uint[[1]] sortnet = generateSortingNetwork (matShape[0]);

    // We will use this offset to decode the sorting network
    uint offset = 0;

    // Extract the number of stages
    uint numOfStages = sortnet[offset++];

    for (uint stage = 0; stage < numOfStages; stage++) {
        uint sizeOfStage = sortnet[offset++];

        D T[[2]] firstVector (sizeOfStage, matShape[1]);
        D T[[2]] secondVector (sizeOfStage, matShape[1]);
        D bool[[1]] exchangeFlagsVector (sizeOfStage);

        // Set up first comparison vector
        for (uint i = 0; i < sizeOfStage; ++i) {
            firstVector[i, :] = matrix[sortnet[offset], :];
            offset++;
        }

        // Set up second comparison vector
        for (uint i = 0; i < sizeOfStage; ++i) {
            secondVector[i, :] = matrix[sortnet[offset], :];
            offset++;
        }

        // Perform compares in parallel as much as possible
        uint colSize = size (firstVector[:, column1]);
        D T[[1]] firstComparisonVector (3 * colSize);
        firstComparisonVector[0:colSize] = firstVector[:, column1];
        firstComparisonVector[colSize:2*colSize] = firstVector[:, column2];
        firstComparisonVector[2*colSize:3*colSize] = secondVector[:, column3];
        D T[[1]] secondComparisonVector (3 * colSize);
        secondComparisonVector[0:colSize] = secondVector[:, column1];
        secondComparisonVector[colSize:2*colSize] = secondVector[:, column2];
        secondComparisonVector[2*colSize:3*colSize] = firstVector[:, column3];
        D bool[[1]] greaterThanResult (3 * colSize);
        greaterThanResult = firstComparisonVector >= secondComparisonVector;

        D T[[1]] firstEqualityVector (2 * colSize);
        firstEqualityVector[0:colSize] = firstVector[:, column1];
        firstEqualityVector[colSize:2*colSize] = firstVector[:, column2];
        D T[[1]] secondEqualityVector (2 * colSize);
        secondEqualityVector[0:colSize] = secondVector[:, column1];
        secondEqualityVector[colSize:2*colSize] = secondVector[:, column2];
        D bool[[1]] equalityResult (3 * colSize);
        equalityResult = firstEqualityVector == secondEqualityVector;

        exchangeFlagsVector = !(greaterThanResult[0:colSize]) || (equalityResult[0:colSize] && (!(greaterThanResult[colSize:2 * colSize]) || (equalityResult[colSize:2 * colSize] && greaterThanResult[2 * colSize:3 * colSize])));

        D bool[[2]] expandedExchangeFlagsVector (2 * sizeOfStage, matShape[1]);

        uint counter = 0;
        for(uint i = 0; i < 2 * sizeOfStage; i = i + 2){
            expandedExchangeFlagsVector[i,:] = exchangeFlagsVector[counter];
            expandedExchangeFlagsVector[i + 1,:] = exchangeFlagsVector[counter];
            counter++;
        }

        // Perform exchanges
        D T[[2]] firstFactor (2 * sizeOfStage, matShape[1]);
        D T[[2]] secondFactor (2 * sizeOfStage, matShape[1]);

        counter = 0;
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {

            firstFactor[i, :] = firstVector[counter, :];
            firstFactor[i + 1, :] = secondVector[counter, :];

            // Comparison bits

            secondFactor[i, :] = secondVector[counter, :];
            secondFactor[i + 1, :] = firstVector[counter, :];
            counter++;
        }

        // Run the largest oblivious choice this side of Dantoiine
        D T[[2]] choiceResults (2 * sizeOfStage, matShape[1]);

        choiceResults = choose(expandedExchangeFlagsVector,firstFactor,secondFactor);

        // Finalize oblivious choices
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {
            matrix[sortnet[offset++], :] = choiceResults [i, :];
        }

        for (uint i = 1; i < (2 * sizeOfStage + 1); i = i + 2) {
            matrix[sortnet[offset++], :] = choiceResults [i, :];
        }

    }
    return matrix;
}
/** @}*/
/** @}*/

/**
 *\cond
 */
bool isPowerOfTwo (uint x) {
    return (x != 0 && (x & (x - 1)) == 0);
}

uint[[1]] generateTopKSortingNetwork (uint n, uint k) {
    assert(isPowerOfTwo (n));

    uint snsize = 0;
    __syscall ("top_k_sorting_network_get_size", n, k, __return snsize);
    uint[[1]] sn(snsize);
    __syscall ("top_k_sorting_network_get", n, k, __ref sn);
    return sn;
}
/**
 *\endcond
 */

/** \addtogroup <selectk>
 *  @{
 *  @brief Functions for selecting k values from a vector/matrix according to an ordering.
 *  @note **D** - all protection domains
 *  @note Supported types \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
*/

/** \addtogroup <selectk_vec>
 *  @{
 *  @brief Function for selecting the k smallest elements of a vector.
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @note Requires that the input array's length is a power of two.
 *  @note The algorithm behind this function is optimized for speed, accuracy is not guaranteed.
 *  @param vector - a vector of supported type
 *  @param k - the number of elements to be selected
 *  @return returns a vector of k elements selected from the input vector
 */

/**
 *\cond
 */
template <domain D : additive3pp>
D uint8[[1]] selectK (D uint8[[1]] vector, uint k) {
    D xor_uint8[[1]] in = reshare (vector);
    D xor_uint8[[1]] out = selectK (in, k);
    return reshare(out);
}

template <domain D : additive3pp>
D uint16[[1]] selectK (D uint16[[1]] vector, uint k) {
    D xor_uint16[[1]] in = reshare (vector);
    D xor_uint16[[1]] out = selectK (in, k);
    return reshare(out);
}

template <domain D : additive3pp>
D uint32[[1]] selectK (D uint32[[1]] vector, uint k) {
    D xor_uint32[[1]] in = reshare (vector);
    D xor_uint32[[1]] out = selectK (in, k);
    return reshare(out);
}

template <domain D : additive3pp>
D uint64[[1]] selectK (D uint64[[1]] vector, uint k) {
    D xor_uint64[[1]] in = reshare (vector);
    D xor_uint64[[1]] out = selectK (in, k);
    return reshare(out);
}

template <domain D : additive3pp>
D int8[[1]] selectK (D int8[[1]] vector, uint k) {
    D uint8[[1]] y = (uint8)vector + 128;
    D xor_uint8[[1]] in = reshare (y);
    D xor_uint8[[1]] out = selectK (in, k);
    y = reshare (out) - 128;
    return (int8)y;
}

template <domain D : additive3pp>
D int16[[1]] selectK (D int16[[1]] vector, uint k) {
    D uint16[[1]] y = (uint16)vector + 32768;
    D xor_uint16[[1]] in = reshare (y);
    D xor_uint16[[1]] out = selectK (in, k);
    y = reshare(out) - 32768;
    return (int16)y;
}

template <domain D : additive3pp>
D int32[[1]] selectK (D int32[[1]] vector, uint k) {
    D uint32[[1]] y = (uint32)vector + 2147483648;
    D xor_uint32[[1]] in = reshare (y);
    D xor_uint32[[1]] out = selectK (in, k);
    y = reshare(out) - 2147483648;
    return (int32)y;
}

template <domain D : additive3pp>
D int64[[1]] selectK (D int64[[1]] vector, uint k) {
    D uint64[[1]] y = (uint64)vector + 9223372036854775808;
    D xor_uint64[[1]] in = reshare (y);
    D xor_uint64[[1]] out = selectK (in, k);
    y = reshare(out) - 9223372036854775808;
    return (int64)y;
}
/**
 *\endcond
 */

template <domain D, type T>
D T[[1]] selectK (D T[[1]] vector, uint k) {
    uint[[1]] sortnet = generateTopKSortingNetwork (size(vector), k);
    uint offset = 0;
    uint numOfStages = sortnet[offset++];

    for (uint stage = 0; stage < numOfStages; stage++) {
        uint sizeOfStage = sortnet[offset++];

        D T[[1]] firstVector (sizeOfStage);
        D T[[1]] secondVector (sizeOfStage);
        D bool[[1]] exchangeFlagsVector (sizeOfStage);

        for (uint i = 0; i < sizeOfStage; i++) {
            firstVector[i] = vector[sortnet[offset]];
            secondVector[i] = vector[sortnet[offset+1]];
            offset += 2;
        }

        exchangeFlagsVector = firstVector <= secondVector;

        D bool[[2]] expandedExchangeFlagsVector (2, sizeOfStage);
        expandedExchangeFlagsVector[0,:] = exchangeFlagsVector;
        expandedExchangeFlagsVector[1,:] = exchangeFlagsVector;

        D T[[2]] firstFactor (2, sizeOfStage);
        D T[[2]] secondFactor (2, sizeOfStage);

        firstFactor[0, :] = firstVector;
        firstFactor[1, :] = secondVector;
        secondFactor[0, :] = secondVector;
        secondFactor[1, :] = firstVector;

        D T[[2]] choiceResults (2, sizeOfStage);

        choiceResults = choose(expandedExchangeFlagsVector,firstFactor,secondFactor);

        offset -= 2 * sizeOfStage;
        for (uint i = 0; i < sizeOfStage; i++) {
            vector[sortnet[offset++]] = choiceResults[0, i];
            vector[sortnet[offset++]] = choiceResults[1, i];
        }
    }

    return vector[:k];
}
/** @} */


/** \addtogroup <selectk_mat>
 *  @{
 *  @brief Function for selecting k rows from a matrix ordered by a column
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref xor_uint8 "xor_uint8" / \ref xor_uint16 "xor_uint16" / \ref xor_uint32 "xor_uint32" / \ref xor_uint64 "xor_uint64"
 *  @note The number of rows of the input matrix has to be a power of two.
 *  @note The algorithm behind this function is optimized for speed, accuracy is not guaranteed.
 *  @param matrix - a matrix of supported type
 *  @param k - number of elements to select
 *  @param column - column to select by
 *  @return returns a matrix with k rows selected from the input vector according to the input column index
 */

/**
 *\cond
 */
template <domain D : additive3pp>
D uint8[[2]] selectK (D uint8[[2]] matrix, uint k, uint column) {
    D xor_uint8[[2]] in = reshare (matrix);
    D xor_uint8[[2]] out = selectK (in, k, column);
    return reshare(out);
}

template <domain D : additive3pp>
D uint16[[2]] selectK (D uint16[[2]] matrix, uint k, uint column) {
    D xor_uint16[[2]] in = reshare (matrix);
    D xor_uint16[[2]] out = selectK (in, k, column);
    return reshare(out);
}

template <domain D : additive3pp>
D uint32[[2]] selectK (D uint32[[2]] matrix, uint k, uint column) {
    D xor_uint32[[2]] in = reshare (matrix);
    D xor_uint32[[2]] out = selectK (in, k, column);
    return reshare(out);
}

template <domain D : additive3pp>
D uint64[[2]] selectK (D uint64[[2]] matrix, uint k, uint column) {
    D xor_uint64[[2]] in = reshare (matrix);
    D xor_uint64[[2]] out = selectK (in, k, column);
    return reshare(out);
}

template <domain D : additive3pp>
D int8[[2]] selectK (D int8[[2]] matrix, uint k, uint column) {
    D uint8[[2]] y = (uint8)matrix + 128;
    D xor_uint8[[2]] in = reshare (y);
    D xor_uint8[[2]] out = selectK (in, k, column);
    y = reshare (out) - 128;
    return (int8)y;
}

template <domain D : additive3pp>
D int16[[2]] selectK (D int16[[2]] matrix, uint k, uint column) {
    D uint16[[2]] y = (uint16)matrix + 32768;
    D xor_uint16[[2]] in = reshare (y);
    D xor_uint16[[2]] out = selectK (in, k, column);
    y = reshare(out) - 32768;
    return (int16)y;
}

template <domain D : additive3pp>
D int32[[2]] selectK (D int32[[2]] matrix, uint k, uint column) {
    D uint32[[2]] y = (uint32)matrix + 2147483648;
    D xor_uint32[[2]] in = reshare (y);
    D xor_uint32[[2]] out = selectK (in, k, column);
    y = reshare(out) - 2147483648;
    return (int32)y;
}

template <domain D : additive3pp>
D int64[[2]] selectK (D int64[[2]] matrix, uint k, uint column) {
    D uint64[[2]] y = (uint64)matrix + 9223372036854775808;
    D xor_uint64[[2]] in = reshare (y);
    D xor_uint64[[2]] out = selectK (in, k, column);
    y = reshare(out) - 9223372036854775808;
    return (int64)y;
}
/**
 *\endcond
 */

template <domain D, type T>
D T[[2]] selectK (D T[[2]] matrix, uint k, uint column) {
    uint[[1]] matShape = shape(matrix);
    uint[[1]] sortnet = generateTopKSortingNetwork (matShape[0], k);
    uint offset = 0;
    uint numOfStages = sortnet[offset++];

    for (uint stage = 0; stage < numOfStages; stage++) {
        uint sizeOfStage = sortnet[offset++];
        D T[[2]] firstVector (sizeOfStage, matShape[1]);
        D T[[2]] secondVector (sizeOfStage, matShape[1]);
        D bool[[1]] exchangeFlagsVector (sizeOfStage);

        for (uint i = 0; i < sizeOfStage; ++i) {
            firstVector[i, :] = matrix[sortnet[offset], :];
            secondVector[i, :] = matrix[sortnet[offset+1], :];
            offset += 2;
        }

        exchangeFlagsVector = firstVector[:, column] <= secondVector[:, column];

        D bool[[2]] expandedExchangeFlagsVector (2 * sizeOfStage, matShape[1]);

        uint counter = 0;
        for(uint i = 0; i < 2 * sizeOfStage; i = i + 2){
            expandedExchangeFlagsVector[i,:] = exchangeFlagsVector[counter];
            expandedExchangeFlagsVector[i + 1,:] = exchangeFlagsVector[counter];
            counter++;
        }

        D T[[2]] firstFactor (2 * sizeOfStage, matShape[1]);
        D T[[2]] secondFactor (2 * sizeOfStage, matShape[1]);

        counter = 0;
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {
            firstFactor[i, :] = firstVector[counter, :];
            firstFactor[i + 1, :] = secondVector[counter, :];

            secondFactor[i, :] = secondVector[counter, :];
            secondFactor[i + 1, :] = firstVector[counter, :];
            counter++;
        }

        D T[[2]] choiceResults (2 * sizeOfStage, matShape[1]);
        choiceResults = choose(expandedExchangeFlagsVector, firstFactor, secondFactor);

        offset -= 2 * sizeOfStage;
        for (uint i = 0; i < 2 * sizeOfStage; i = i + 2) {
            matrix[sortnet[offset++], :] = choiceResults[i, :];
            matrix[sortnet[offset++], :] = choiceResults[i+1, :];
        }
    }

    return matrix;
}
/** @} */
/** @} */
/** @}*/

