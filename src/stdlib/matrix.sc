/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */




/// \todo Need vecLength and unitVector, but waiting for public sqrt.
/// \todo ? inverse, det, eigenvectors?
/**
* \cond
*/
module matrix;

import stdlib;
/**
* \endcond
*/
/**
* @file
* \defgroup matrix matrix.sc
* \defgroup transpose transpose
* \defgroup rowsums rowSums
* \defgroup unitmatrix unitMatrix
* \defgroup colsums colSums
* \defgroup crossproduct crossProduct
* \defgroup dotproduct dotProduct
* \defgroup dotproduct_vec dotProduct[[1]]
* \defgroup dotproduct_mat dotProduct[[2]]
* \defgroup crossproduct_vec crossProduct[[1]]
* \defgroup crossproduct_mat crossProduct[[2]]
* \defgroup matrixmultiplication matrixMultiplication
* \defgroup determinant determinant
*/

/*******************************
	transpose
********************************/

/** \addtogroup <matrix> 
*@{
*
* @brief Module with functions for manipulating matrices and vectors
*/

/** \addtogroup <transpose> 
 *  @{
 *  @brief Function for transposing matrices
 *  @note **D** - all protection domains
 *  @note **T** - any \ref data_types "data" type
 */

/**
* @param mat - a 2-dimensional matrix
* @return returns the transposed version of the input matrix
*/
template <domain D, type T>
D T[[2]] transpose (D T[[2]] mat) {
	uint[[1]] matShape = shape (mat);
	D T[[2]] result (matShape[1], matShape[0]);

	for(uint i = 0; i < matShape[1]; ++i) {
		result[i, :] = mat[:, i];
	}
	return result;
}

/**
* @param arr - a 3-dimensional array
* @note Transposes across the last two dimensions
* @return returns a 2-dimensional array transposed across the last two dimension
*/ 
template <domain D, type T>
D T[[3]] transpose (D T[[3]] arr) {	
	uint[[1]] matShape = shape (arr);
	D T[[3]] result (matShape[0], matShape[2], matShape[1]);

	for(uint j = 0; j < matShape[0]; ++j) {
		for(uint i = 0; i < matShape[1]; ++i) {
			result[j, :, i] = arr[j, i, :];
		}
	}
	return result;
}

/** @}*/

/*******************************
      Unit matrix
********************************/

/** \addtogroup <unitmatrix> 
 *  @{
 *  @brief Function for creating a unit matrix
 *  @note **D** - all protection domains
 *  @param x - an \ref uint64 "uint64"/\ref uint64 "uint" type parameter, for specifying the size of the unit matrix
 *  @return returns an uint64/uint type unit matrix of size x  
 */

template <domain D>
D uint[[2]] unitMatrix(D uint x){
	D uint[[2]] mat (x,x);
	for(uint i = 0; i < x; ++i){
		mat[i,i] = 1;
	}
	return mat;
}

/** @}*/	

/*******************************
	rowSums, colSums
********************************/

/** \addtogroup <rowsums> 
 *  @{
 *  @brief Function for summarizing the rows of a matrix
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a matrix of supported type
 *  @return returns a vector with the sums of each row in the input matrix
*/

template <domain D>
D uint[[1]] rowSums (D bool[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D uint8[[1]] rowSums (D uint8[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint8[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D uint16[[1]] rowSums (D uint16[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint16[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D uint32[[1]] rowSums (D uint32[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint32[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D uint[[1]] rowSums (D uint[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D int8[[1]] rowSums (D int8[[2]] mat) {
	uint rows = shape(mat)[0];
	D int8[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D int16[[1]] rowSums (D int16[[2]] mat) {
	uint rows = shape(mat)[0];
	D int16[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D int32[[1]] rowSums (D int32[[2]] mat) {
	uint rows = shape(mat)[0];
	D int32[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D int[[1]] rowSums (D int[[2]] mat) {
	uint rows = shape(mat)[0];
	D int[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D float32[[1]] rowSums (D float32[[2]] mat) {
	uint rows = shape(mat)[0];
	D float32[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D>
D float64[[1]] rowSums (D float64[[2]] mat) {
	uint rows = shape(mat)[0];
	D float64[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

/** @}*/
/** \addtogroup <colsums> 
 *  @{
 *  @brief Function for summarizing the columns of a matrix
 *  @note **D** - all protection domains
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a matrix of supported type
 *  @return returns a vector with the sums of each column in the input matrix
 */

template <domain D>
D uint[[1]] colSums (D bool[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D uint8[[1]] colSums (D uint8[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D uint16[[1]] colSums (D uint16[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D uint32[[1]] colSums (D uint32[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D uint[[1]] colSums (D uint[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D int8[[1]] colSums (D int8[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D int16[[1]] colSums (D int16[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D int32[[1]] colSums (D int32[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D int[[1]] colSums (D int[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D float32[[1]] colSums (D float32[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D>
D float64[[1]] colSums (D float64[[2]] mat) {
	return rowSums(transpose(mat));
}

/** @}*/

/*******************************
	dotProduct
********************************/

/** \addtogroup <dotproduct> 
 *  @{
 *  @brief Function for finding the dot product of two vectors/matrices
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 */

/** \addtogroup <dotproduct_vec> 
 *  @{
 *  @brief Function for finding the dot product of two vectors
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @return returns a scalar with the dot product of the two input vectors
 */ 

template <domain D>
D uint8 dotProduct (D uint8[[1]] x, D uint8[[1]] y) {
	assert (size (x) == size (y));
	D uint8[[1]] product = x * y;
	return sum (product);
}

template <domain D>
D uint16 dotProduct (D uint16[[1]] x, D uint16[[1]] y) {
	assert (size (x) == size (y));
	D uint16[[1]] product = x * y;
	return sum (product);
}

template <domain D>
D uint32 dotProduct (D uint32[[1]] x, D uint32[[1]] y) {
	assert (size (x) == size (y));
	D uint32[[1]] product = x * y;
	return sum (product);
}

template <domain D>
D uint dotProduct (D uint[[1]] x, D uint[[1]] y) {
	assert (size (x) == size (y));
	D uint[[1]] product = x * y;
	return sum (product);
}

template <domain D>
D int8 dotProduct (D int8[[1]] x, D int8[[1]] y) {
	assert (size (x) == size (y));
	D int8[[1]] product = x * y;
	return sum (product);
}

template <domain D>
D int16 dotProduct (D int16[[1]] x, D int16[[1]] y) {
	assert (size (x) == size (y));
	D int16[[1]] product = x * y;
	return sum (product);
}

template <domain D>
D int32 dotProduct (D int32[[1]] x, D int32[[1]] y) {
	assert (size (x) == size (y));
	D int32[[1]] product = x * y;
	return sum (product);
}

template <domain D>
D int dotProduct (D int[[1]] x, D int[[1]] y) {
	assert (size (x) == size (y));
	D int[[1]] product = x * y;
	return sum (product);
}

template <domain D>
D float32 dotProduct (D float32[[1]] x, D float32[[1]] y) {
	assert (size (x) == size (y));
	D float32[[1]] product = x * y;
	return sum (product);
}

template <domain D>
D float64 dotProduct (D float64[[1]] x, D float64[[1]] y) {
	assert (size (x) == size (y));
	D float64[[1]] product = x * y;
	return sum (product);
}

/** @}*/
/** \addtogroup <dotproduct_mat> 
 *  @{
 *  @brief Function for finding the dot product of two matrices
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param x,y - matrices of supported type
 *  @return returns a vector with the dot product of each row of the two input matrices
 */ 

template <domain D>
D uint8[[1]] dotProduct (D uint8[[2]] x, D uint8[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D uint8[[2]] product = x * y;
	D uint8[[1]] result = rowSums(product);

	return result;
}

template <domain D>
D uint16[[1]] dotProduct (D uint16[[2]] x, D uint16[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D uint16[[2]] product = x * y;
	D uint16[[1]] result = rowSums(product);

	return result;
}

template <domain D>
D uint32[[1]] dotProduct (D uint32[[2]] x, D uint32[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D uint32[[2]] product = x * y;
	D uint32[[1]] result = rowSums(product);

	return result;
}

template <domain D>
D uint[[1]] dotProduct (D uint[[2]] x, D uint[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D uint[[2]] product = x * y;
	D uint[[1]] result = rowSums(product);

	return result;
}

template <domain D>
D int8[[1]] dotProduct (D int8[[2]] x, D int8[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D int8[[2]] product = x * y;
	D int8[[1]] result = rowSums(product);

	return result;
}

template <domain D>
D int16[[1]] dotProduct (D int16[[2]] x, D int16[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D int16[[2]] product = x * y;
	D int16[[1]] result = rowSums(product);

	return result;
}

template <domain D>
D int32[[1]] dotProduct (D int32[[2]] x, D int32[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D int32[[2]] product = x * y;
	D int32[[1]] result = rowSums(product);

	return result;
}

template <domain D>
D int[[1]] dotProduct (D int[[2]] x, D int[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D int[[2]] product = x * y;
	D int[[1]] result = rowSums(product);

	return result;
}

template <domain D>
D float32[[1]] dotProduct (D float32[[2]] x, D float32[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D float32[[2]] product = x * y;
	D float32[[1]] result = rowSums(product);

	return result;
}

template <domain D>
D float64[[1]] dotProduct (D float64[[2]] x, D float64[[2]] y) {
	assert (all (shape(x) == shape(y)));

	D float64[[2]] product = x * y;
	D float64[[1]] result = rowSums(product);

	return result;
}

/** @}*/
/** @}*/

/*******************************
	crossProduct
********************************/

/** \addtogroup <crossproduct> 
 *  @{
 *  @brief Function for finding the cross product of two vectors/matrices
 *  @note **D** - all protection domains
 *  @note Supported types - \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 */


 /** \addtogroup <crossproduct_vec> 
 *  @{
 *  @brief Function for finding the cross product of two vectors
 *  @note **D** - all protection domains
 *  @note Supported types - \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param x,y - vectors of supported type
 *  @return returns a vector with the cross product of the two input vectors
 */ 

template <domain D>
D int8[[1]] crossProduct (D int8[[1]] x, D int8[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D int8[[1]] result (3);

	result[0] = x[1] * y[2] - x[2] * y[1];
	result[1] = x[2] * y[0] - x[0] * y[2];
	result[2] = x[0] * y[1] - x[1] * y[0];

	return result;
}

template <domain D>
D int16[[1]] crossProduct (D int16[[1]] x, D int16[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D int16[[1]] result (3);

	result[0] = x[1] * y[2] - x[2] * y[1];
	result[1] = x[2] * y[0] - x[0] * y[2];
	result[2] = x[0] * y[1] - x[1] * y[0];

	return result;
}

template <domain D>
D int32[[1]] crossProduct (D int32[[1]] x, D int32[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D int32[[1]] result (3);

	result[0] = x[1] * y[2] - x[2] * y[1];
	result[1] = x[2] * y[0] - x[0] * y[2];
	result[2] = x[0] * y[1] - x[1] * y[0];

	return result;
}

template <domain D>
D int[[1]] crossProduct (D int[[1]] x, D int[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D int[[1]] result (3);

	result[0] = x[1] * y[2] - x[2] * y[1];
	result[1] = x[2] * y[0] - x[0] * y[2];
	result[2] = x[0] * y[1] - x[1] * y[0];

	return result;
}

template <domain D>
D float32[[1]] crossProduct (D float32[[1]] x, D float32[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D float32[[1]] result (3);

	result[0] = x[1] * y[2] - x[2] * y[1];
	result[1] = x[2] * y[0] - x[0] * y[2];
	result[2] = x[0] * y[1] - x[1] * y[0];

	return result;
}

template <domain D>
D float64[[1]] crossProduct (D float64[[1]] x, D float64[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D float64[[1]] result (3);

	result[0] = x[1] * y[2] - x[2] * y[1];
	result[1] = x[2] * y[0] - x[0] * y[2];
	result[2] = x[0] * y[1] - x[1] * y[0];

	return result;
}

/** @}*/
/** \addtogroup <crossproduct_mat> 
 *  @{
 *  @brief Function for finding the cross product of two matrices
 *  @note **D** - all protection domains
 *  @note Supported types - \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param x,y - matrices of supported type
 *  @return returns a matrix with the cross product of each row of the two input matrices
 */ 


template <domain D>
D int8[[2]] crossProduct (D int8[[2]] x, D int8[[2]] y) {
	uint[[1]] xShape = shape (x);
	assert(xShape[1] == 3 && all(xShape == shape (y)));
	D int8[[2]] result (xShape[0], xShape[1]);

	for (uint i = 0; i < xShape[0]; ++i){
		result[i,:] = crossProduct (x[i,:], y[i,:]);		
	}

	return result;
}

template <domain D>
D int16[[2]] crossProduct (D int16[[2]] x, D int16[[2]] y) {
	uint[[1]] xShape = shape (x);
	assert(xShape[1] == 3 && all(xShape == shape (y)));
	D int16[[2]] result (xShape[0], xShape[1]);

	for (uint i = 0; i < xShape[0]; ++i){
		result[i,:] = crossProduct (x[i,:], y[i,:]);
	}

	return result;
}

template <domain D>
D int32[[2]] crossProduct (D int32[[2]] x, D int32[[2]] y) {
	uint[[1]] xShape = shape (x);
	assert(xShape[1] == 3 && all(xShape == shape (y)));
	D int32[[2]] result (xShape[0], xShape[1]);

	for (uint i = 0; i < xShape[0]; ++i){
		result[i,:] = crossProduct (x[i,:], y[i,:]);
	}

	return result;
}

template <domain D>
D int[[2]] crossProduct (D int[[2]] x, D int[[2]] y) {
	uint[[1]] xShape = shape (x);
	assert(xShape[1] == 3 && all(xShape == shape (y)));
	D int[[2]] result (xShape[0], xShape[1]);

	for (uint i = 0; i < xShape[0]; ++i){
		result[i,:] = crossProduct (x[i,:], y[i,:]);
	}

	return result;
}

template <domain D>
D float32[[2]] crossProduct (D float32[[2]] x, D float32[[2]] y) {
	uint[[1]] xShape = shape (x);
	assert(xShape[1] == 3 && all(xShape == shape (y)));
	D float32[[2]] result (xShape[0], xShape[1]);

	for (uint i = 0; i < xShape[0]; ++i){
		result[i,:] = crossProduct (x[i,:], y[i,:]);
	}

	return result;
}

template <domain D>
D float64[[2]] crossProduct (D float64[[2]] x, D float64[[2]] y) {
	uint[[1]] xShape = shape (x);
	assert(xShape[1] == 3 && all(xShape == shape (y)));
	D float64[[2]] result (xShape[0], xShape[1]);

	for (uint i = 0; i < xShape[0]; ++i){
		result[i,:] = crossProduct (x[i,:], y[i,:]);
	}

	return result;
}

/** @}*/
/** @}*/


/*******************************
	matrixMultiplication
********************************/

/** \addtogroup <matrixmultiplication> 
 *  @{
 *  @brief Function for multiplying two matrices
 *  @note **D** - all protection domains
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @warning no. of columns of x must equal no. of rows of y
 *  @param x,y - matrices of supported type and shape
 *  @return returns the matrix of x*y
 */
 template <domain D>
D uint8[[2]] matrixMultiplication (D uint8[[2]] x, D uint8[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D uint8[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

template <domain D>
D uint16[[2]] matrixMultiplication (D uint16[[2]] x, D uint16[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D uint16[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

template <domain D>
D uint32[[2]] matrixMultiplication (D uint32[[2]] x, D uint32[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D uint32[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

template <domain D>
D uint[[2]] matrixMultiplication (D uint[[2]] x, D uint[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D uint[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

template <domain D>
D int8[[2]] matrixMultiplication (D int8[[2]] x, D int8[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D int8[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

template <domain D>
D int16[[2]] matrixMultiplication (D int16[[2]] x, D int16[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D int16[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

template <domain D>
D int32[[2]] matrixMultiplication (D int32[[2]] x, D int32[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D int32[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

template <domain D>
D int[[2]] matrixMultiplication (D int[[2]] x, D int[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D int[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

template <domain D>
D float32[[2]] matrixMultiplication (D float32[[2]] x, D float32[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D float32[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

template <domain D>
D float64[[2]] matrixMultiplication (D float64[[2]] x, D float64[[2]] y) {
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	assert (xShape[1] == yShape[0]);

	D float64[[2]] result (xShape[0], yShape[1]);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	for(uint i = 0; i < yShape[1]; ++i) {
		for(uint j = 0; j < xShape[0]; j++) {
			result[j,i] = dotProduct(x[j, :], y[:, i]);
		}
	}

	return result;
}

/** @}*/
/** \addtogroup <determinant> 
 *  @{
 *  @brief Function for finding the determinant of a matrix
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 *  @param mat - a matrix of supported type
 *  @return returns the determinant of the input matrix
 */

/* Determinant using LUP-decomposition method. */
float32 determinant (float32[[2]] mat) {
	uint[[1]] s = shape(mat);
	assert(s[0] == s[1]);
	uint n = s[0];

	int exchanges = 0;

   	for (uint k = 0; k < n; ++k) {
   		float32 p = 0;
   		uint kprim;
   		for (uint i = k; i < n; ++i) {
   			if (abs(mat[i,k]) > p) {
   				p = abs(mat[i,k]);
   				kprim = i;
   			} 
   		}
   		if (p == 0) {
   			return 0;
   		}
   		
   		if (k != kprim) {
   			++exchanges;

	   		float32[[1]] tmp1 = mat[k,:];
	   		mat[k,:] = mat[kprim,:];
	   		mat[kprim,:] = tmp1;
	   	}

   		for (uint i = k+1; i < n; ++i) {
   			mat[i,k] /= mat[k,k];
   			for (uint j = k+1; j < n; ++j) {
   				mat[i,j] -= mat[i,k]*mat[k,j];
   			}
   		}
   		
   	}

   	float32 det = 1;   	

   	for (uint i = 0; i < n; ++i) {
   		det *= mat[i,i];   		
   	}

   	if (exchanges % 2 == 1) {
   		det = -det;
   	}   	

   	return det;
}

float64 determinant (float64[[2]] mat) {
	uint[[1]] s = shape(mat);
	assert(s[0] == s[1]);
	uint n = s[0];

	int exchanges = 0;

   	for (uint k = 0; k < n; ++k) {
   		float64 p = 0;
   		uint kprim;
   		for (uint i = k; i < n; ++i) {
   			if (abs(mat[i,k]) > p) {
   				p = abs(mat[i,k]);
   				kprim = i;
   			} 
   		}
   		if (p == 0) {
   			return 0;
   		}
   		
   		if (k != kprim) {
   			++exchanges;

	   		float64[[1]] tmp1 = mat[k,:];
	   		mat[k,:] = mat[kprim,:];
	   		mat[kprim,:] = tmp1;
	   	}

   		for (uint i = k+1; i < n; ++i) {
   			mat[i,k] /= mat[k,k];
   			for (uint j = k+1; j < n; ++j) {
   				mat[i,j] -= mat[i,k]*mat[k,j];
   			}
   		}
   		
   	}

   	float64 det = 1;   	

   	for (uint i = 0; i < n; ++i) {
   		det *= mat[i,i];   		
   	}

   	if (exchanges % 2 == 1) {
   		det = -det;
   	}   	

   	return det;
}


/**
* @note **D** - all protection domains
* @note naive determinant implementation
*/
template <domain D>
D float64 determinant(D float32[[2]] mat) {
	uint[[1]] s = shape(mat);
	assert(s[0] == s[1]);
	uint n = s[0];
	assert (n > 0);

    D float64 det;

	if (n == 1) {
	  	det = mat[0, 0];
	} else if (n == 2) {
	  	det = mat[0,0] * mat[1,1] - mat[1,0] * mat[0,1];
	} else {
		D float64[[2]] minor (n-1, n-1);
	  	for (uint j1 = 0; j1 < n; ++j1) {		     	
	     	for (uint i = 1; i < n; ++i) {
	        	uint j2 = 0;
	        	for (uint j = 0; j < n; ++j) {
	           		if (j == j1)
	              		continue;
	           		minor[i-1, j2] = mat[i, j];
	           		++j2;
	        	}
	     	}
	     	bool isEven = j1 % 2 == 0;
	    	det += isEven ? mat[0, j1] * determinant(minor) : -1 * mat[0, j1] * determinant(minor);	     
	    }
	}
	return det;
}

/**
* @note **D** - all protection domains
* @note naive determinant implementation
*/
template <domain D>
D float64 determinant(D float64[[2]] mat) {
	uint[[1]] s = shape(mat);
	assert(s[0] == s[1]);
	uint n = s[0];
	assert (n > 0);

    D float64 det;

	if (n == 1) {
	  	det = mat[0, 0];
	} else if (n == 2) {
	  	det = mat[0,0] * mat[1,1] - mat[1,0] * mat[0,1];
	} else {
		D float64[[2]] minor (n-1, n-1);
	  	for (uint j1 = 0; j1 < n; ++j1) {		     	
	     	for (uint i = 1; i < n; ++i) {
	        	uint j2 = 0;
	        	for (uint j = 0; j < n; ++j) {
	           		if (j == j1)
	              		continue;
	           		minor[i-1, j2] = mat[i, j];
	           		++j2;
	        	}
	     	}
	     	bool isEven = j1 % 2 == 0;
	    	det += isEven ? mat[0, j1] * determinant(minor) : -1 * mat[0, j1] * determinant(minor);	     
	    }
	}
	return det;
}


// for reference - creates P*mat = L*U in-place
// P - permutation matrix
// L - lower unit diagonal matrix
// U - upper diagonal matrix
// returns L and U in a single matrix, also calculates P, but doesn't return it

/**
* \cond
*/
float64[[2]] LupDecomposition (float64[[2]] mat) {
	uint[[1]] s = shape(mat);
	assert(s[0] == s[1]);
	uint n = s[0];

	uint[[1]] perm = (uint)mat[0,:];
   	for (uint i = 0; i < n; ++i) {
   		perm[i] = i;
   	}

   	for (uint k = 0; k < n; ++k) {
   		float64 p = 0;
   		uint kprim;
   		for (uint i = k; i < n; ++i) {
   			if (abs(mat[i,k]) > p) {
   				p = abs(mat[i,k]);
   				kprim = i;
   			} 
   		}
   		if (p == 0) {
   			print("error singular mat");
   			return mat;
   		}
   		
   		if (k != kprim) {
   			uint tmp = perm[k];
	   		perm[k] = perm[kprim];
	   		perm[kprim] = tmp;

	   		float64[[1]] tmp1 = mat[k,:];
	   		mat[k,:] = mat[kprim,:];
	   		mat[kprim,:] = tmp1;
	   	}

   		for (uint i = k+1; i < n; ++i) {
   			mat[i,k] /= mat[k,k];
   			for (uint j = k+1; j < n; ++j) {
   				mat[i,j] -= mat[i,k]*mat[k,j];
   			}
   		}
   		
   	}
   	
   	return mat;
}

/**
* \endcond
*/
/** @}*/
/** @}*/