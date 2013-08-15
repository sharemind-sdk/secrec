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
module stdlib;
/**
* \endcond
*/

uint8 UINT8_MAX = 255;
uint8 UINT8_MIN = 0;


uint16 UINT16_MAX = 65535;
uint16 UINT16_MIN = 0;


uint32 UINT32_MAX = 4294967295;
uint32 UINT32_MIN = 0;


uint64 UINT64_MAX = 18446744073709551615;
uint64 UINT64_MIN = 0;


int8 INT8_MAX = 127;
int8 INT8_MIN = -128;


int16 INT16_MAX = 32767;
int16 INT16_MIN = -32768;


int32 INT32_MAX = 2147483647;
int32 INT32_MIN = -2147483648;


int64 INT64_MAX = 9223372036854775807;
int64 INT64_MIN = -9223372036854775808;


/**
* @file
* \defgroup stdlib stdlib.sc
* \defgroup flatten flatten
* \defgroup shapesareequal shapesAreEqual
* \defgroup arraytostring arrayToString
* \defgroup printvector printVector
* \defgroup printmatrix printMatrix
* \defgroup print3darray print3dArray
* \defgroup printarray printArray
* \defgroup any any
* \defgroup any1 any
* \defgroup any2 any(parts)
* \defgroup all all
* \defgroup all1 all
* \defgroup all2 all(parts)
* \defgroup sum sum
* \defgroup sum1 sum
* \defgroup sum2 sum(parts)
* \defgroup product product
* \defgroup product1 product
* \defgroup product2 product(parts)
* \defgroup max max
* \defgroup max1 max
* \defgroup max2 max(pointwise)
* \defgroup max3 max(parts)
* \defgroup min min
* \defgroup min1 min
* \defgroup min2 min(pointwise)
* \defgroup min3 min(parts)
* \defgroup abs abs
* \defgroup round round
*/

/** \addtogroup <stdlib> 
*@{
* @brief Module with standard general functions
*/


/*******************************
	Utility - flattening
********************************/

/** \addtogroup <flatten> 
 *  @{
 *  @brief Function for converting arrays to 1 dimensional
 *  @note **D** - all protection domains
 *  @note **N** - any array size of any dimension
 *  @note **T** - any \ref data_types "data" type
 *  @return returns a 1-dimensional flattened version of the input array with the same type
 *  @return For example: {{1,2,3}{4,5,6}} -> {1,2,3,4,5,6}
 */

template <domain D, type T, dim N>
D T[[1]] flatten (D T[[N]] arr) {
	return reshape(arr,size(arr));
}

/** @}*/

/*******************************
	Utility - shapes are equal
********************************/

/** \addtogroup <shapesareequal> 
 *  @{
 *  @brief Function for checking the shape equality of two arrays
 *  @note **D1** - all protection domains
 *  @note **D2** - all protection domains
 *  @note **N** - any array size of any dimension
 *  @note **T1** - any \ref data_types "data" type
 *  @note **T2** - any \ref data_types "data" type
 *  @return returns a \ref bool "bool" type value, whether the input arrays are of equal shape (**true**) or are not of equal shape (**false**)
 */

template <domain D1, domain D2, type T1, type T2, dim N>
bool shapesAreEqual(D1 T1[[N]] first, D2 T2[[N]] second) {
    uint[[1]] s1 = shape(first);
    uint[[1]] s2 = shape(second);
    uint n = size(s1);
    for (uint i = 0; i < n; ++i) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }

    return true;
}

/** @}*/

/*******************************
	Pretty printing
********************************/
/**
* \cond
*/
template <type T>
string _vectorToString (T[[1]] vec) {
	uint s = size (vec);
	if (s == 0) {
		return "{}";
	}
	uint sm = s - 1;
  	string output = "{";
  	for (uint i = 0; i < sm; ++i) {
    	output += tostring(vec[i]) + ", ";
  	}
  	output += tostring(vec[sm]) + "}";
  	return output;
}

/**
* \endcond
*/

/** \addtogroup <arraytostring> 
 *  @{
 *  @brief Function for converting an array to a string
 *  @note **D** - all protection domains
 *  @note **T** - any \ref data_types "data" type
 *  @return returns a string representation of the input array
 */


/**
*  @param scalar - a scalar of any type
*/
template <type T>
string arrayToString (T scalar) {
	return tostring(scalar);
}

/**
*  @param vec - a 1-dimension array of any type
*/

template <type T>
string arrayToString (T[[1]] vec) {
	return _vectorToString(vec);
}

/**
*  @param arr - any dimension array of any type
*/

template <type T, dim N>
string arrayToString (T[[N]] arr) {
	uint n = size(arr);
	if (n == 0) {
		return "{}";
	}
	T[[1]] flatArr = reshape(arr, n);

	return _vectorToString(flatArr);
}

/** @}*/

/** \addtogroup <printvector> 
 *  @{
 *  @brief Function for printing out vectors
 *  @note **T** - any \ref data_types "data" type
 *  @param vec - a 1-dimensional array
 *  @return prints out a string representation of the input vector
 *  @return see also \ref printarray "printArray" 
 */

template <type T>
void printVector (T[[1]] vec) {
	print(_vectorToString(vec));
}

/** @}*/

/** \addtogroup <printmatrix> 
 *  @{
 *  @brief Function for printing out matrices
 *  @note **T** - any \ref data_types "data" type
 *  @param mat - a 2-dimensional matrix
 *  @return prints out a string representation of the input matrix.
 *  @return see also \ref printarray "printArray"
 */

// nicer version of 2-dimensional printArray
template <type T>
void printMatrix (T[[2]] mat) {
	uint rows = shape (mat)[0];	
	if (rows == 0) {
		print("{}");
	} else {
		if (rows == 1) {
			print("{ " + _vectorToString(mat[0,:]) + " }");
		} else {
			uint rowsm = rows-1;
			print("{ " + _vectorToString(mat[0,:]) + ",");
			for (uint i = 1; i < rowsm; ++i) {
				print("  " + _vectorToString(mat[i,:]) + ",");
			}
			print("  " + _vectorToString(mat[rowsm,:]) + " }");
		}
	}	
}

/** @}*/
/** \addtogroup <print3darray> 
 *  @{
 *  @brief Function for printing out 3-dimensional arrays
 *  @note **T** - any \ref data_types "data" type
 *  @param arr - a 3-dimensional array
 *  @return prints out a string representation of the input array. 
 *  @return see also \ref printarray "printArray"
 */

template <type T>
void print3dArray (T[[3]] arr) {
	uint[[1]] s = shape(arr);
	uint matrices = s[0];
	uint rows = s[1];	
	if (matrices == 0) {
		print("{}");
	} else {
		print("{");
		printMatrix(arr[0,:,:]);
		if (matrices > 1) {
			print(",");
			uint matricesm = matrices-1;
			for (uint i = 1; i < matricesm; ++i) {
				printMatrix(arr[i,:,:]);
				print(",");
			}
			printMatrix(arr[matricesm,:,:]);
		}		
		print("}");		
	}
}

/** @}*/
/** \addtogroup <printarray> 
 *  @{
 *  @brief Function for printing out any dimension arrays
 *  @note **T** - any \ref data_types "data" type
 *  @param arr - any dimension array
 *  @return prints out a string representation of the input array.
 *  @return see also \ref printvector "printVector" / \ref printmatrix "printMatrix" / \ref print3darray "print3dArray"
 */

template <type T, dim N>
void printArray (T[[N]] arr) {
	print("shape: " + _vectorToString(shape(arr)));
	print("elements: " + arrayToString(arr));
}
/** @}*/

/*******************************
	All, any
********************************/


/** \addtogroup <any> 
 *  @{
 *  @brief Function for checking if any element in a boolean vector is true
 *  @note **D** - all protection domains
 */

/** \addtogroup <any1> 
 *  @{
 *  @brief Function for checking if any element in a boolean vector is true
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool"
 *  @return **true** if any of the input bits are set
 *  @return **false** if all input bits are not set
 *  @note iteratively checks all elements in input vector and returns true when the first 1-bit is found.
 */

/**
*  @param b - boolean scalar
*/
template <domain D>
D bool any (D bool b) {
	return b;	
}

/**
*  \cond
*/
bool any (bool[[1]] vec) {
	uint n = size(vec);
	for (uint i = 0; i<n; ++i) {
		if (vec[i]) {
			return true;
		}
	}
    return false;
}

/**
*  \endcond
*  @param vec - boolean 1-dimensional vector
*/
template <domain D>
D bool any (D bool[[1]] vec) {
	uint n = size(vec);
	D bool result = false;
	for (uint i = 0; i<n; ++i) {
		result = result || vec[i];
	}
    return result;
}

/**
*  @param arr - boolean any dimension array
*/
template <domain D, dim N>
D bool any (D bool[[N]] arr) {
	return any( flatten(arr) );
}


/** @}*/
/** \addtogroup <any2> 
 *  @{
 *  @brief Function for checking if any element in a boolean vector is true in specified parts
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool"
 *  @param vec - a vector of supported type
 *  @param k - an \ref uint64 "uint" type value for specifying from how many subarrays **any** must be found
 *  @return returns a boolean vector that evaluates every subarray seperately for \ref any1 "any"
 *  @note iteratively checks all elements in input vector and returns true when the first 1-bit is found.
 */

/**
* \cond
*/
bool[[1]] any (bool[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	uint len = n/k;
	bool[[1]] result (k);
	for (uint i = 0; i<k; ++i) {
		for (uint j = 0; j<len; ++j) {
			if (vec[i*len+j]) {
				result[i] = true;
				break;
			}
		}
	}
	
    return result;
}

/**
* \endcond
*/
template <domain D>
D bool[[1]] any (D bool[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	uint len = n/k;
	D bool[[1]] result (k);
	for (uint i = 0; i<k; ++i) {
		for (uint j = 0; j<len; ++j) {
			result[i] = result[i] || vec[i*len+j];
		}
	}
    return result;
}


/** @}*/
/** @}*/
/** \addtogroup <all> 
 *  @{
 *  @brief Function for checking if all elements in a boolean vector are true
 *  @note **D** - all protection domains
 */

/** \addtogroup <all1> 
 *  @{
 *  @brief Function for checking if all elements in a boolean vector are true
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool"
 *  @return **true** if all of the input bits are set
 *  @return **false** if any input bit is not set
 *  @note iteratively checks all elements in input vector and returns false when the first 0-bit is found.
 */

/**
*  @param b - boolean scalar
*/
template <domain D>
D bool all (D bool b) {
    return b;   
}

/**
* \cond
*/
bool all (bool[[1]] vec) {
    uint n = size (vec);
    for (uint i = 0; i<n; ++i) {
        if (!vec[i]) {
            return false;
        }
    }
    return true;
}

/**
*  \endcond
*  @param vec - boolean 1-dimensional vector
*/
template <domain D>
D bool all (D bool[[1]] vec) {
    uint n = size (vec);
    D bool result = true;
    for (uint i = 0; i<n; ++i) {
        result = result && vec[i];
    }
    return result;
}

/**
*  @param arr - boolean any dimension array
*/
template <domain D, dim N>
D bool all (D bool[[N]] arr) {
    return all( flatten(arr) );
}

/** @}*/
/** \addtogroup <all2> 
 *  @{
 *  @brief Function for checking if all elements in a boolean vector are true in specified parts
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool"
 *  @param vec - a vector of supported type
 *  @param k - an \ref uint64 "uint" type value for specifying from how many subarrays **all** must be found
 *  @return returns a boolean vector that evaluates every subarray seperately for \ref all1 "all"
 *  @note iteratively checks all elements in input vector and returns false when the first 0-bit is found.
 */

/**
* \cond
*/
bool[[1]] all (bool[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	uint len = n/k;
	bool[[1]] result (k) = true;
	for (uint i = 0; i<k; ++i) {
		for (uint j = 0; j<len; ++j) {
			if (!vec[i*len+j]) {
				result[i] = false;
				break;
			}
		}
	}
	
    return result;
}

/**
* \endcond
*/

template <domain D>
D bool[[1]] all (D bool[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	uint len = n/k;
	D bool[[1]] result (k) = true;
	for (uint i = 0; i<k; ++i) {
		for (uint j = 0; j<len; ++j) {
			result[i] = result[i] && vec[i*len+j];
		}
	}
    return result;
}

/** @}*/
/** @}*/



/*******************************
	Sum
********************************/


/** \addtogroup <sum> 
 *  @{
 *  @brief Function for finding the sum of all elements in the input vector
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 */

/** \addtogroup <sum1> 
 *  @{
 *  @brief Function for finding the sum of all elements in the input vector
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 * @return returns the sum of all elements in input vector
 * @note uses accumulator to calculate sum. May be very inefficient for private domains.
 */

template <domain D, type T>
D T sum (D T scalar) {
	return scalar; 
}

template <domain D>
D uint sum (D bool[[1]] vec) {
	return sum((uint)vec);
}

template <domain D>
D uint8 sum (D uint8[[1]] vec) {
	uint n = size(vec);
	D uint8 sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

template <domain D>
D uint16 sum (D uint16[[1]] vec) {
	uint n = size(vec);
	D uint16 sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

template <domain D>
D uint32 sum (D uint32[[1]] vec) {
	uint n = size(vec);
	D uint32 sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

template <domain D>
D uint sum (D uint[[1]] vec) {
	uint n = size(vec);
	D uint sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

template <domain D>
D int8 sum (D int8[[1]] vec) {
	uint n = size(vec);
	D int8 sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

template <domain D>
D int16 sum (D int16[[1]] vec) {
	uint n = size(vec);
	D int16 sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

template <domain D>
D int32 sum (D int32[[1]] vec) {
	uint n = size(vec);
	D int32 sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

template <domain D>
D int sum (D int[[1]] vec) {
	uint n = size(vec);
	D int sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

template <domain D>
D float32 sum (D float32[[1]] vec) {
	uint n = size(vec);
	D float32 sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

template <domain D>
D float64 sum (D float64[[1]] vec) {
	uint n = size(vec);
	D float64 sumOfArr;
	for (uint i = 0; i<n; ++i) {
		sumOfArr += vec[i];
	}
	return sumOfArr;
}

/** @}*/
/** \addtogroup <sum2> 
 *  @{
 *  @brief Function for finding the sum of all elements in the input vector in specified parts
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 * @param k - an \ref uint64 "uint" type scalar which specifies in how many parts the sum is found. \n
 	For example if k = 2 then the input vector is split into two parts and the sums of those parts are found seperately.
 * @return returns a vector with the sum of the specified number of parts in the input vector
 * @note uses accumulator to calculate sum. May be very inefficient for private domains.
 */

template <domain D>
D uint[[1]] sum (D bool[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D uint[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D uint8[[1]] sum (D uint8[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D uint8[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D uint16[[1]] sum (D uint16[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D uint16[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D uint32[[1]] sum (D uint32[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D uint32[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D uint[[1]] sum (D uint[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D uint[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D int8[[1]] sum (D int8[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D int8[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D int16[[1]] sum (D int16[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D int16[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D int32[[1]] sum (D int32[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D int32[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D int[[1]] sum (D int[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D int[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D float32[[1]] sum (D float32[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D float32[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}

template <domain D>
D float64[[1]] sum (D float64[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	D float64[[1]] sumsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		sumsOfSubArrs[i] = sum(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return sumsOfSubArrs;
}
/** @}*/
/** @}*/


/*******************************
	Product
********************************/

/** \addtogroup <product> 
 *  @{
 *  @brief Function for finding the product of all elements in the input vector
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 */

/** \addtogroup <product1> 
 *  @{
 *  @brief Function for finding the product of the input vector
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 * @return returns the product of the input vector
 */

template <domain D, type T>
D T product (D T scalar) {
	return scalar;
}


template <domain D>
D uint8 product (D uint8[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	uint8 result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

template <domain D>
D uint16 product (D uint16[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	uint16 result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

template <domain D>
D uint32 product (D uint32[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	uint32 result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

template <domain D>
D uint product (D uint[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	uint result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

template <domain D>
D int8 product (D int8[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	int8 result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

template <domain D>
D int16 product (D int16[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	int16 result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

template <domain D>
D int32 product (D int32[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	int32 result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

template <domain D>
D int product (D int[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	int result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

template <domain D>
D float32 product (D float32[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	float32 result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

template <domain D>
D float64 product (D float64[[1]] vec) {
	uint n = size(vec);
	if (n == 0) {
		return 0;
	} 

	float64 result = 1;
	for (uint i = 0; i<n; ++i) {
		result *= vec[i];
	}
	return result;
}

/** @}*/
/** \addtogroup <product2> 
 *  @{
 *  @brief Function for finding the product of the input vector in parts
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 * @param k - an \ref uint64 "uint" type scalar which specifies in how many parts the product is found. \n
 	For example if k = 2 then the input vector is split into two parts and the products of those parts are found seperately.
 * @return returns a vector with the product of the specified number of parts in the input vector
 */
template <domain D>
D uint8[[1]] product (D uint8[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D uint8[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

template <domain D>
D uint16[[1]] product (D uint16[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D uint16[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

template <domain D>
D uint32[[1]] product (D uint32[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D uint32[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

template <domain D>
D uint[[1]] product (D uint[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D uint[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

template <domain D>
D int8[[1]] product (D int8[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D int8[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

template <domain D>
D int16[[1]] product (D int16[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D int16[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

template <domain D>
D int32[[1]] product (D int32[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D int32[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

template <domain D>
D int[[1]] product (D int[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D int[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

template <domain D>
D float32[[1]] product (D float32[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D float32[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

template <domain D>
D float64[[1]] product (D float64[[1]] vec, uint k) {
	uint n = size(vec);
	assert(k > 0 && n % k == 0);
	
	D float64[[1]] prodsOfSubArrs (k);
	uint subArrLen = n/k;
	uint subArrStartIdx = 0;
	for (uint i = 0; i<k; ++i) {
		prodsOfSubArrs[i] = product(vec[subArrStartIdx : subArrStartIdx+subArrLen]);
		subArrStartIdx += subArrLen;
	}
	return prodsOfSubArrs;
}

/** @}*/
/** @}*/

/*******************************
	Minimum, maximum
********************************/



/** \addtogroup <min> 
 *  @{
 *  @brief Function for finding the minimum values
 */

 /** \addtogroup <min1> 
 *  @{
 *  @brief Function for finding the minimum value of the input
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" 
 *  @return returns the minimum value of the input
 */

/**
* @note **D** - all protection domains
* @param scalar - a scalar value
* @return returns the scalar value
*/
template <domain D, type T>
D T min (D T scalar) {
	return scalar;
}

/**
* \cond
*/
//does not work for floats, xor_uints
template <domain D, type T>
D T min (T x, T y) {
	D T isSmaller = (T) x < y;
	return isSmaller*x + (1-isSmaller)*y;
}

template <domain D>
D bool min (D bool[[1]] vec) {
	return all(vec);
}

/**
* \endcond
*/

/**
* @param vec - a 1-dimensional vector
* @return returns the smallest value in the vector
*/
template <type T>
T min (T[[1]] vec) {
	uint n = size(vec);
	assert(n > 0);
	T result = vec[0];
	T tmp;
	for (uint i = 1; i<n; ++i) {
		tmp = vec[i];
		if (tmp < result) {
			result = tmp;
		}
	}
	return result;
}

/**
* @note does not work for floats, xor_uints
* @note **D** - all protection domains
* @param vec - a 1-dimensional vector
* @return returns the smallest value in the vector
*/
template <domain D, type T>
D T min (D T[[1]] vec) {
	uint n = size(vec);
	assert(n > 0);
	D T result = vec[0];
	D T tmp;
	for (uint i = 1; i<n; ++i) {
		tmp = vec[i];
		D T isSmaller = (T) (tmp < result);
		result -= isSmaller*result;
		result += isSmaller*tmp;
	}
	return result;
}

/**
* @param arr - an array of any dimension
* @return returns the smallest value in the array
*/
template <type T, dim N>
T min (T[[N]] arr) {
	return min(flatten(arr));
}

/** @}*/
 /** \addtogroup <min2> 
 *  @{
 *  @brief Function for finding the pointwise minimum value of the two input vectors
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" 
 *  @param x,y - input vector of supported type
 *  @return returns the pointwise minimum of the two input vectors
 *  \cond
 */

/**
* \cond
*/

/** pointwise minimum */
template <type T>
T min (T x, T y) {
	return x < y ? x : y;
}

template <domain D>
D bool min (D bool x, D bool y) {
	return x && y;
}

template <domain D>
D bool[[1]] min (D bool[[1]] x, D bool[[1]] y) {
	assert(size(x) == size(y));
	return x && y;
}

template <type T>
T[[1]] min (T[[1]] x, T[[1]] y) {
	uint n = size(x);
	assert(n == size(y));
	for (uint i = 0; i<n; ++i) {
		if (y[i] < x[i]) {
			x[i] = y[i];
		}
	}
	return x;
}

/**
* \endcond
*/

//does not work for floats, xor_uints
template <domain D, type T>
D T[[1]] min (D T[[1]] x, D T[[1]] y) {
	uint n = size(x);
	assert(n == size(y));
	for (uint i = 0; i<n; ++i) {
		D T isSmaller = (T) x[i] < y[i];
		x[i] = isSmaller*x[i] + (1-isSmaller)*y[i];
	}
	return x;
}


 /** @}*/
 /** \addtogroup <min3> 
 *  @{
 *  @brief Function for finding the minimum value of the input in specified parts
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" 
 *  @param vec - input vector on supported type
 *  @param k - an \ref uint64 "uint" type value for specifying from how many subarrays **min** must be found
 *  @return returns the minimum value of every subarray in the input
 *  \cond
 */
template <domain D>
D bool[[1]] min (D bool[[1]] vec, uint k) {
	return all(vec,k);
}

template <type T>
T[[1]] min (T[[1]] vec, uint k) {
	uint n = size(vec);
	assert(n > 0 && n % k == 0);
	uint len = n/k;
	T[[1]] result (k);
	for (uint i = 0; i < k; ++i) {
		result[i] = vec[i*len];
		T tmp;
		for (uint j = 1; j<len; ++j) {
			tmp = vec[i*len+j];
			if (tmp < result[i]) {
				result[i] = tmp;
			}
		}
	}
	return result;
}

/**
* \endcond
*/

//does not work for floats, xor_uints
template <domain D, type T>
D T[[1]] min (D T[[1]] vec, uint k) {
	uint n = size(vec);
	assert(n > 0 && n % k == 0);
	uint len = n/k;
	D T[[1]] result (k);
	for (uint i = 0; i < k; ++i) {
		result[i] = vec[i*len];
		D T tmp;
		for (uint j = 1; j<len; ++j) {
			tmp = vec[i*len+j];
			D T isSmaller = (T) (tmp < result[i]);
			result[i] -= isSmaller*result[i];
			result[i] += isSmaller*tmp;
		}
	}
	return result;
}



/** @}*/
/** @}*/
/** \addtogroup <max> 
 *  @{
 *  @brief Function for finding the maximum value of the input
 */

 /** \addtogroup <max1> 
 *  @{
 *  @brief Function for finding the maximum value of the input
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" 
 *  @return return the maximum value of the input
 */

/**
* @note **D** - all protection domains
* @param scalar - a scalar value
* @return returns the scalar value
*/
template <domain D, type T>
D T max (D T scalar) {
	return scalar;
}

/**
* @note **D** - all protection domains
* @param vec - a 1-dimensional vector
* @return returns the largest value in the vector
* \cond
*/
template <type T>
T max (T[[1]] vec) {
	uint n = size(vec);
	assert(n > 0);
	T result = vec[0];
	T tmp;
	for (uint i = 1; i<n; ++i) {
		tmp = vec[i];
		if (tmp > result) {
			result = tmp;
		}
	}
	return result;
}

/**
* \endcond
*/

//does not work for floats, xor_uints
/**
* @note **D** - all protection domains
* @param vec - a 1-dimensional vector
* @return returns the largest value in the vector
* \cond
*/
template <domain D, type T>
D T max (D T[[1]] vec) {
	uint n = size(vec);
	assert(n > 0);
	D T result = vec[0];
	D T tmp;
	for (uint i = 1; i<n; ++i) {
		tmp = vec[i];
		D T isLarger = (T) (tmp > result);
		result -= isLarger*result;
		result += isLarger*tmp;
	}
	return result;
}

/**
* @param arr - an array of any dimension
* @return returns the largest value in the array
*/
template <type T, dim N>
T max (T[[N]] arr) {
	return max(flatten(arr));
}

/**
* \cond
*/
template <domain D>
D bool max (D bool[[1]] vec) {
	return any(vec);
}

bool max (bool[[1]] vec) {
	return any(vec);
}

/**
* \endcond
*/

/** @}*/
 /** \addtogroup <max2> 
 *  @{
 *  @brief Function for finding the pointwise maximum of two input vectors
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" 
 *  @param x,y - input vectors of supported type
 *  @return return the pointwise maximum of the two input vectors
 */

/**
* \cond
*/
/* pointwise maximum */
template <type T>
T max (T x, T y) {
	return x > y ? x : y;
}

template <type T>
T[[1]] max (T[[1]] x, T[[1]] y) {
	uint n = size(x);
	assert(n == size(y));
	for (uint i = 0; i<n; ++i) {
		if (y[i] > x[i]) {
			x[i] = y[i];
		}
	}
	return x;
}

template <domain D>
D bool max (D bool x, D bool y) {
	return x || y;
}

//does not work for floats, xor_uints
template <domain D, type T>
D T max (T x, T y) {
	D T isLarger = (T) x > y;
	return isLarger*x + (1-isLarger)*y;
}

template <domain D>
D bool[[1]] max (D bool[[1]] x, D bool[[1]] y) {
	assert(size(x) == size(y));
	return x || y;
}
/**
* \endcond
*/
//does not work for floats, xor_uints
template <domain D, type T>
D T[[1]] max (D T[[1]] x, D T[[1]] y) {
	uint n = size(x);
	assert(n == size(y));
	for (uint i = 0; i<n; ++i) {
		D T isLarger = (T) x[i] > y[i];
		x[i] = isLarger*x[i] + (1-isLarger)*y[i];
	}
	return x;
}
/** @}*/
/** \addtogroup <max3> 
*  @{
*  @brief Function for finding the maximum value of the input in specified parts
*  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" 
*  @param vec - a vector of supported type
*  @param k - an \ref uint64 "uint" type value for specifying from how many subarrays **max** must be found
*  @return returns the maximum value of every subarray in the input
*/

/**
* \cond
*/
bool[[1]] max (bool[[1]] vec, uint k) {
	return any(vec,k);
}

template <type T>
T[[1]] max (T[[1]] vec, uint k) {
	uint n = size(vec);
	assert(n > 0 && n % k == 0);
	uint len = n/k;
	T[[1]] result (k);
	for (uint i = 0; i < k; ++i) {
		result[i] = vec[i*len];
		T tmp;
		for (uint j = 1; j<len; ++j) {
			tmp = vec[i*len+j];
			if (tmp > result[i]) {
				result[i] = tmp;
			}
		}
	}
	return result;
}

template <domain D>
D bool[[1]] max (D bool[[1]] vec, uint k) {
	return any(vec,k);
}
/**
* \endcond
*/
//does not work for floats, xor_uints
template <domain D, type T>
D T[[1]] max (D T[[1]] vec, uint k) {
	uint n = size(vec);
	assert(n > 0 && n % k == 0);
	uint len = n/k;
	D T[[1]] result (k);
	for (uint i = 0; i < k; ++i) {
		result[i] = vec[i*len];
		D T tmp;
		for (uint j = 1; j<len; ++j) {
			tmp = vec[i*len+j];
			D T isLarger = (T) (tmp > result[i]);
			result[i] -= isLarger*result[i];
			result[i] += isLarger*tmp;
		}
	}
	return result;
}

/** @}*/
/** @}*/
/** \addtogroup <abs> 
*  @{
*  @brief Function for finding the absolute value of the input
*  @note **D** - all protection domains
*  @note Supported types - \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64" 
*  @param x - input scalar of supported type
*  @return returns the absolute value of the input
*  \todo
*/
template <domain D>
D int8 abs (D int8 x) {
	return x < 0 ? -x : x;
}

template <domain D>
D int16 abs (D int16 x) {
	return x < 0 ? -x : x;
}

template <domain D>
D int32 abs (D int32 x) {
	return x < 0 ? -x : x;
}

template <domain D>
D int abs (D int x) {
	return x < 0 ? -x : x;
}

float32 abs (float32 x) {
	return x < 0 ? -x : x;
}

float64 abs (float64 x) {	
	return x < 0 ? -x : x;
}

/** @}*/
/** \addtogroup <round> 
*  @{
*  @brief Function for rounding values
*  @note Supported types - \ref float32 "float32" / \ref float64 "float64" 
*  @param x - input scalar of supported type
*  @return returns the rounded value of the input scalar
* \todo what if argument is bigger than INT64_MAX?
*/
int round (float32 x) {
	float32 k = (float32)((int)x);
	return x - k < 0.5 ? (int)k : ((int)k) + 1;
}

int round (float64 x) {
	float64 k = (float64)((int)x);
	return x - k < 0.5 ? (int)k : ((int)k) + 1;
}

/** @}*/
/** @}*/


/** @}*/