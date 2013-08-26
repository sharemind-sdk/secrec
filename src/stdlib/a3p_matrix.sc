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
module a3p_matrix;

import stdlib;
import additive3pp;
import matrix;
/**
* \endcond
*/
/**
* @file
* \defgroup a3p_matrix a3p_matrix.sc
* \defgroup a3p_rowsums rowSums
* \defgroup a3p_colsums colSums
* \defgroup a3p_dotproduct dotProduct
* \defgroup a3p_dotproduct_vec dotProduct[[1]]
* \defgroup a3p_dotproduct_mat dotProduct[[2]]
* \defgroup a3p_veclength vecLength
* \defgroup a3p_unitvector unitVector
* \defgroup a3p_crossproduct crossProduct
* \defgroup a3p_crossproduct_vec crossProduct[[1]]
* \defgroup a3p_crossproduct_mat crossProduct[[2]]
* \defgroup a3p_matrixmultiplication matrixMultiplication
* \defgroup a3p_matrixmultiplication_mat matrixMultiplication[[2]]
* \defgroup a3p_matrixmultiplication_cube matrixMultiplication[[3]]
* \defgroup a3p_diag_matrixmultiplication diagMatrixMultiplication
* \defgroup a3p_diag_matrixmultiplication_mat diagMatrixMultiplication[[2]]
* \defgroup a3p_diag_matrixmultiplication_cube diagMatrixMultiplication[[3]]
*/

/** \addtogroup <a3p_matrix>
*@{
*
* @brief Module with functions for manipulating matrices and vectors (additive3pp protection domain)
*/

/*******************************
	rowSums, colSums
********************************/

/** \addtogroup <a3p_rowsums>
 *  @{
 *  @brief Function for summarizing the rows of a matrix
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @note When adding boolean values, the numerical value of boolean is used
 *  @param mat - a matrix of supported type
 *  @return returns a vector with the sums of each row in the input matrix
 */

template <domain D : additive3pp>
D uint[[1]] rowSums (D bool[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D uint8[[1]] rowSums (D uint8[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint8[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D uint16[[1]] rowSums (D uint16[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint16[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D uint32[[1]] rowSums (D uint32[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint32[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D uint[[1]] rowSums (D uint[[2]] mat) {
	uint rows = shape(mat)[0];
	D uint[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D int8[[1]] rowSums (D int8[[2]] mat) {
	uint rows = shape(mat)[0];
	D int8[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D int16[[1]] rowSums (D int16[[2]] mat) {
	uint rows = shape(mat)[0];
	D int16[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D int32[[1]] rowSums (D int32[[2]] mat) {
	uint rows = shape(mat)[0];
	D int32[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D int[[1]] rowSums (D int[[2]] mat) {
	uint rows = shape(mat)[0];
	D int[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D float32[[1]] rowSums (D float32[[2]] mat) {
	uint rows = shape(mat)[0];
	D float32[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

template <domain D : additive3pp>
D float64[[1]] rowSums (D float64[[2]] mat) {
	uint rows = shape(mat)[0];
	D float64[[1]] sumsOfRows = sum(flatten(mat),rows);
	return sumsOfRows;
}

/** @}*/
/** \addtogroup <a3p_colsums>
 *  @{
 *  @brief Function for summarizing the columns of a matrix
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @note When adding boolean values, the numerical value of boolean is used
 *  @param mat - a matrix of supported type
 *  @return returns a vector with the sums of each column in the input matrix
 */

template <domain D : additive3pp>
D uint[[1]] colSums (D bool[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D uint8[[1]] colSums (D uint8[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D uint16[[1]] colSums (D uint16[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D uint32[[1]] colSums (D uint32[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D uint[[1]] colSums (D uint[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D int8[[1]] colSums (D int8[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D int16[[1]] colSums (D int16[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D int32[[1]] colSums (D int32[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D int[[1]] colSums (D int[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D float32[[1]] colSums (D float32[[2]] mat) {
	return rowSums(transpose(mat));
}

template <domain D : additive3pp>
D float64[[1]] colSums (D float64[[2]] mat) {
	return rowSums(transpose(mat));
}

/** @}*/

/*******************************
	dotProduct
********************************/

/** \addtogroup <a3p_dotproduct>
 *  @{
 *  @brief Function for finding the dot product of two vectors/matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 */

/** \addtogroup <a3p_dotproduct_vec>
 *  @{
 *  @brief Function for finding the dot product of two vectors
 *  @note **D** - additive3pp domain
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @return returns a scalar with the dot product of the two input vectors
 */

template <domain D : additive3pp>
D uint dotProduct (D bool[[1]] x, D bool[[1]] y) {
	assert (size (x) == size (y));
	D bool[[1]] product = x && y;
	return sum (product);
}

template <domain D : additive3pp>
D uint8 dotProduct (D uint8[[1]] x, D uint8[[1]] y) {
	assert (size (x) == size (y));
	D uint8[[1]] product = x * y;
	return sum (product);
}

template <domain D : additive3pp>
D uint16 dotProduct (D uint16[[1]] x, D uint16[[1]] y) {
	assert (size (x) == size (y));
	D uint16[[1]] product = x * y;
	return sum (product);
}

template <domain D : additive3pp>
D uint32 dotProduct (D uint32[[1]] x, D uint32[[1]] y) {
	assert (size (x) == size (y));
	D uint32[[1]] product = x * y;
	return sum (product);
}

template <domain D : additive3pp>
D uint dotProduct (D uint[[1]] x, D uint[[1]] y) {
	assert (size (x) == size (y));
	D uint[[1]] product = x * y;
	return sum (product);
}

template <domain D : additive3pp>
D int8 dotProduct (D int8[[1]] x, D int8[[1]] y) {
	assert (size (x) == size (y));
	D int8[[1]] product = x * y;
	return sum (product);
}

template <domain D : additive3pp>
D int16 dotProduct (D int16[[1]] x, D int16[[1]] y) {
	assert (size (x) == size (y));
	D int16[[1]] product = x * y;
	return sum (product);
}

template <domain D : additive3pp>
D int32 dotProduct (D int32[[1]] x, D int32[[1]] y) {
	assert (size (x) == size (y));
	D int32[[1]] product = x * y;
	return sum (product);
}

template <domain D : additive3pp>
D int dotProduct (D int[[1]] x, D int[[1]] y) {
	assert (size (x) == size (y));
	D int[[1]] product = x * y;
	return sum (product);
}

template <domain D : additive3pp>
D float32 dotProduct (D float32[[1]] x, D float32[[1]] y) {
	assert (size (x) == size (y));
	D float32[[1]] product = x * y;
	return sum (product);
}

template <domain D : additive3pp>
D float64 dotProduct (D float64[[1]] x, D float64[[1]] y) {
	assert (size (x) == size (y));
	D float64[[1]] product = x * y;
	return sum (product);
}

/** @}*/
/** \addtogroup <a3p_dotproduct_mat>
 *  @{
 *  @brief Function for finding the dot product of two matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref bool "bool" / \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param x,y - matrices of supported type
 *  @return returns a vector with the dot product of each row of the two input matrices
 */

template <domain D : additive3pp>
D bool[[1]] dotProduct (D bool[[2]] x, D bool[[2]] y) {
	assert (shapesAreEqual(x,y));

	D bool[[2]] product = x && y;
	D bool[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D uint8[[1]] dotProduct (D uint8[[2]] x, D uint8[[2]] y) {
	assert (shapesAreEqual(x,y));

	D uint8[[2]] product = x * y;
	D uint8[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D uint16[[1]] dotProduct (D uint16[[2]] x, D uint16[[2]] y) {
	assert (shapesAreEqual(x,y));

	D uint16[[2]] product = x * y;
	D uint16[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D uint32[[1]] dotProduct (D uint32[[2]] x, D uint32[[2]] y) {
	assert (shapesAreEqual(x,y));

	D uint32[[2]] product = x * y;
	D uint32[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D uint[[1]] dotProduct (D uint[[2]] x, D uint[[2]] y) {
	assert (shapesAreEqual(x,y));

	D uint[[2]] product = x * y;
	D uint[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D int8[[1]] dotProduct (D int8[[2]] x, D int8[[2]] y) {
	assert (shapesAreEqual(x,y));

	D int8[[2]] product = x * y;
	D int8[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D int16[[1]] dotProduct (D int16[[2]] x, D int16[[2]] y) {
	assert (shapesAreEqual(x,y));

	D int16[[2]] product = x * y;
	D int16[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D int32[[1]] dotProduct (D int32[[2]] x, D int32[[2]] y) {
	assert (shapesAreEqual(x,y));

	D int32[[2]] product = x * y;
	D int32[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D int[[1]] dotProduct (D int[[2]] x, D int[[2]] y) {
	assert (shapesAreEqual(x,y));

	D int[[2]] product = x * y;
	D int[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D float32[[1]] dotProduct (D float32[[2]] x, D float32[[2]] y) {
	assert (shapesAreEqual(x,y));

	D float32[[2]] product = x * y;
	D float32[[1]] result = rowSums(product);

	return result;
}

template <domain D : additive3pp>
D float64[[1]] dotProduct (D float64[[2]] x, D float64[[2]] y) {
	assert (shapesAreEqual(x,y));

	D float64[[2]] product = x * y;
	D float64[[1]] result = rowSums(product);

	return result;
}
/** @}*/
/** @}*/

/*******************************
	vecLength, unitVector
********************************/

/** \addtogroup <a3p_veclength>
 *  @{
 *  @brief Function for finding the length of a vector
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 */

/**
*  @param x - vector of supported type
*  @return returns the length of the vector
*/
template <domain D : additive3pp>
D float32 vecLength (D float32[[1]] x) {
	return sqrt (dotProduct (x, x));
}

/**
*  @param x - vector of supported type
*  @return returns the length of the vector
*/
template <domain D : additive3pp>
D float64 vecLength (D float64[[1]] x) {
	return sqrt (dotProduct (x, x));
}

/**
*  @param x - matrix of supported type
*  @return returns a vector with length of each row in the matrix
*/
template <domain D : additive3pp>
D float32[[1]] vecLength (D float32[[2]] x) {
	return sqrt (dotProduct (x, x));
}

/**
*  @param x - matrix of supported type
*  @return returns a vector with length of each row in the matrix
*/
template <domain D : additive3pp>
D float64[[1]] vecLength (D float64[[2]] x) {
	return sqrt (dotProduct (x, x));
}

/** @}*/
/** \addtogroup <a3p_unitvector>
 *  @{
 *  @brief Function for finding the unit vector of the input vector
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref float32 "float32" / \ref float64 "float64"
 */

/**
*  @param x - vector of supported type
*  @return returns the unit vector for the input vector
*/
template <domain D : additive3pp>
D float32[[1]] unitVector (D float32[[1]] x) {
	assert(size(x) > 0);
	D float32 veclen = vecLength(x);
	return (x / veclen);
}

/**
*  @param x - vector of supported type
*  @return returns the unit vector for the input vector
*/
template <domain D : additive3pp>
D float64[[1]] unitVector (D float64[[1]] x) {
	assert(size(x) > 0);
	D float64 veclen = vecLength(x);
	return (x / veclen);
}

/**
*  @param x - matrix of supported type
*  @return returns a matrix with the unit vector of each row in the input matrix
*/
template <domain D : additive3pp>
D float32[[2]] unitVector (D float32[[2]] x) {
	assert(shape(x)[1] > 0);
	D float32[[1]] len = vecLength (x);
	// Expand len
	uint[[1]] xShape = shape (x);
	D float32[[2]] lenExpanded (xShape[0], xShape[1]);
	for (uint i = 0; i < xShape[0]; ++i) {
		lenExpanded[i, :] = len[i];
	}
	return (x/lenExpanded);
}

/**
*  @param x - matrix of supported type
*  @return returns a matrix with the unit vector of each row in the input matrix
*/
template <domain D : additive3pp>
D float64[[2]] unitVector (D float64[[2]] x) {
	assert(shape(x)[1] > 0);
	D float64[[1]] len = vecLength (x);
	// Expand len
	uint[[1]] xShape = shape (x);
	D float64[[2]] lenExpanded (xShape[0], xShape[1]);
	for (uint i = 0; i < xShape[0]; ++i) {
		lenExpanded[i, :] = len[i];
	}
	return (x/lenExpanded);
}
/** @}*/



/*******************************
	crossProduct
********************************/
/** \addtogroup <a3p_crossproduct>
 *  @{
 *  @brief Function for finding the cross product of two vectors/matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 */


 /** \addtogroup <a3p_crossproduct_vec>
 *  @{
 *  @brief Function for finding the cross product of two vectors
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param x,y - vectors of supported type
 *  @return returns a vector with the cross product of the two input vectors
 */


template <domain D : additive3pp>
D int8[[1]] crossProduct (D int8[[1]] x, D int8[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D int8[[1]] result (3);
	D int8[[1]] parProdA (6), parProdB (6), parProdRes (6);

	parProdA[0] = x[1];
	parProdB[0] = y[2];
	parProdA[3] = x[2];
	parProdB[3] = y[1];
	parProdA[1] = x[2];
	parProdB[1] = y[0];
	parProdA[4] = x[0];
	parProdB[4] = y[2];
	parProdA[2] = x[0];
	parProdB[2] = y[1];
	parProdA[5] = x[1];
	parProdB[5] = y[0];
	parProdRes = parProdA * parProdB;

	result = parProdRes[0 : 3] - parProdRes[3 : 6];
	return result;
}

template <domain D : additive3pp>
D int16[[1]] crossProduct (D int16[[1]] x, D int16[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D int16[[1]] result (3);
	D int16[[1]] parProdA (6), parProdB (6), parProdRes (6);

	parProdA[0] = x[1];
	parProdB[0] = y[2];
	parProdA[3] = x[2];
	parProdB[3] = y[1];
	parProdA[1] = x[2];
	parProdB[1] = y[0];
	parProdA[4] = x[0];
	parProdB[4] = y[2];
	parProdA[2] = x[0];
	parProdB[2] = y[1];
	parProdA[5] = x[1];
	parProdB[5] = y[0];
	parProdRes = parProdA * parProdB;

	result = parProdRes[0 : 3] - parProdRes[3 : 6];
	return result;
}

template <domain D : additive3pp>
D int32[[1]] crossProduct (D int32[[1]] x, D int32[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D int32[[1]] result (3);
	D int32[[1]] parProdA (6), parProdB (6), parProdRes (6);

	parProdA[0] = x[1];
	parProdB[0] = y[2];
	parProdA[3] = x[2];
	parProdB[3] = y[1];
	parProdA[1] = x[2];
	parProdB[1] = y[0];
	parProdA[4] = x[0];
	parProdB[4] = y[2];
	parProdA[2] = x[0];
	parProdB[2] = y[1];
	parProdA[5] = x[1];
	parProdB[5] = y[0];
	parProdRes = parProdA * parProdB;

	result = parProdRes[0 : 3] - parProdRes[3 : 6];
	return result;
}

template <domain D : additive3pp>
D int[[1]] crossProduct (D int[[1]] x, D int[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D int[[1]] result (3);
	D int[[1]] parProdA (6), parProdB (6), parProdRes (6);

	parProdA[0] = x[1];
	parProdB[0] = y[2];
	parProdA[3] = x[2];
	parProdB[3] = y[1];
	parProdA[1] = x[2];
	parProdB[1] = y[0];
	parProdA[4] = x[0];
	parProdB[4] = y[2];
	parProdA[2] = x[0];
	parProdB[2] = y[1];
	parProdA[5] = x[1];
	parProdB[5] = y[0];
	parProdRes = parProdA * parProdB;

	result = parProdRes[0 : 3] - parProdRes[3 : 6];
	return result;
}

template <domain D : additive3pp>
D float32[[1]] crossProduct (D float32[[1]] x, D float32[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D float32[[1]] result (3);
	D float32[[1]] parProdA (6), parProdB (6), parProdRes (6);

	parProdA[0] = x[1];
	parProdB[0] = y[2];
	parProdA[3] = x[2];
	parProdB[3] = y[1];
	parProdA[1] = x[2];
	parProdB[1] = y[0];
	parProdA[4] = x[0];
	parProdB[4] = y[2];
	parProdA[2] = x[0];
	parProdB[2] = y[1];
	parProdA[5] = x[1];
	parProdB[5] = y[0];
	parProdRes = parProdA * parProdB;

	result = parProdRes[0 : 3] - parProdRes[3 : 6];
	return result;
}


template <domain D : additive3pp>
D float64[[1]] crossProduct (D float64[[1]] x, D float64[[1]] y) {
	assert(size(x) == 3 && size(y) == 3);
	D float64[[1]] result (3);
	D float64[[1]] parProdA (6), parProdB (6), parProdRes (6);

	parProdA[0] = x[1];
	parProdB[0] = y[2];
	parProdA[3] = x[2];
	parProdB[3] = y[1];
	parProdA[1] = x[2];
	parProdB[1] = y[0];
	parProdA[4] = x[0];
	parProdB[4] = y[2];
	parProdA[2] = x[0];
	parProdB[2] = y[1];
	parProdA[5] = x[1];
	parProdB[5] = y[0];
	parProdRes = parProdA * parProdB;

	result = parProdRes[0 : 3] - parProdRes[3 : 6];
	return result;
}

/** @}*/
/** \addtogroup <a3p_crossproduct_mat>
 *  @{
 *  @brief Function for finding the cross product of two matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @param x,y - matrices of supported type
 *  @return returns a matrix with the cross product of each row of the two input matrices
 */

template <domain D : additive3pp>
D int8[[2]] crossProduct (D int8[[2]] x, D int8[[2]] y) {

	uint[[1]] xShape = shape (x);
	uint[[1]] yShape = shape (y);

	assert(xShape[1] == 3 && yShape[1] == 3 && xShape[0] == yShape[0]);
	D int8[[2]] result (xShape[0], xShape[1]);

	D int8[[2]] parProdA (xShape[0], xShape[1] * 2), parProdB (xShape[0], xShape[1] * 2), parProdRes (xShape[0], xShape[1] * 2);

	for (uint i = 0; i < xShape[0]; i++){
		parProdA[i, 0] = x[i, 1];
		parProdB[i, 0] = y[i, 2];
		parProdA[i, 3] = x[i, 2];
		parProdB[i, 3] = y[i, 1];
		parProdA[i, 1] = x[i, 2];
		parProdB[i, 1] = y[i, 0];
		parProdA[i, 4] = x[i, 0];
		parProdB[i, 4] = y[i, 2];
		parProdA[i, 2] = x[i, 0];
		parProdB[i, 2] = y[i, 1];
		parProdA[i, 5] = x[i, 1];
		parProdB[i, 5] = y[i, 0];
	}

	parProdRes = parProdA * parProdB;
	result = parProdRes[:, 0 : 3] - parProdRes[:, 3 : 6];
	return result;
}



template <domain D : additive3pp>
D int16[[2]] crossProduct (D int16[[2]] x, D int16[[2]] y) {

	uint[[1]] xShape = shape (x);
	uint[[1]] yShape = shape (y);

	assert(xShape[1] == 3 && yShape[1] == 3 && xShape[0] == yShape[0]);
	D int16[[2]] result (xShape[0], xShape[1]);

	D int16[[2]] parProdA (xShape[0], xShape[1] * 2), parProdB (xShape[0], xShape[1] * 2), parProdRes (xShape[0], xShape[1] * 2);

	for (uint i = 0; i < xShape[0]; i++){
		parProdA[i, 0] = x[i, 1];
		parProdB[i, 0] = y[i, 2];
		parProdA[i, 3] = x[i, 2];
		parProdB[i, 3] = y[i, 1];
		parProdA[i, 1] = x[i, 2];
		parProdB[i, 1] = y[i, 0];
		parProdA[i, 4] = x[i, 0];
		parProdB[i, 4] = y[i, 2];
		parProdA[i, 2] = x[i, 0];
		parProdB[i, 2] = y[i, 1];
		parProdA[i, 5] = x[i, 1];
		parProdB[i, 5] = y[i, 0];
	}

	parProdRes = parProdA * parProdB;
	result = parProdRes[:, 0 : 3] - parProdRes[:, 3 : 6];
	return result;
}



template <domain D : additive3pp>
D int32[[2]] crossProduct (D int32[[2]] x, D int32[[2]] y) {

	uint[[1]] xShape = shape (x);
	uint[[1]] yShape = shape (y);

	assert(xShape[1] == 3 && yShape[1] == 3 && xShape[0] == yShape[0]);
	D int32[[2]] result (xShape[0], xShape[1]);

	D int32[[2]] parProdA (xShape[0], xShape[1] * 2), parProdB (xShape[0], xShape[1] * 2), parProdRes (xShape[0], xShape[1] * 2);

	for (uint i = 0; i < xShape[0]; i++){
		parProdA[i, 0] = x[i, 1];
		parProdB[i, 0] = y[i, 2];
		parProdA[i, 3] = x[i, 2];
		parProdB[i, 3] = y[i, 1];
		parProdA[i, 1] = x[i, 2];
		parProdB[i, 1] = y[i, 0];
		parProdA[i, 4] = x[i, 0];
		parProdB[i, 4] = y[i, 2];
		parProdA[i, 2] = x[i, 0];
		parProdB[i, 2] = y[i, 1];
		parProdA[i, 5] = x[i, 1];
		parProdB[i, 5] = y[i, 0];
	}

	parProdRes = parProdA * parProdB;
	result = parProdRes[:, 0 : 3] - parProdRes[:, 3 : 6];
	return result;
}



template <domain D : additive3pp>
D int[[2]] crossProduct (D int[[2]] x, D int[[2]] y) {

	uint[[1]] xShape = shape (x);
	uint[[1]] yShape = shape (y);

	assert(xShape[1] == 3 && yShape[1] == 3 && xShape[0] == yShape[0]);
	D int[[2]] result (xShape[0], xShape[1]);

	D int[[2]] parProdA (xShape[0], xShape[1] * 2), parProdB (xShape[0], xShape[1] * 2), parProdRes (xShape[0], xShape[1] * 2);

	for (uint i = 0; i < xShape[0]; i++){
		parProdA[i, 0] = x[i, 1];
		parProdB[i, 0] = y[i, 2];
		parProdA[i, 3] = x[i, 2];
		parProdB[i, 3] = y[i, 1];
		parProdA[i, 1] = x[i, 2];
		parProdB[i, 1] = y[i, 0];
		parProdA[i, 4] = x[i, 0];
		parProdB[i, 4] = y[i, 2];
		parProdA[i, 2] = x[i, 0];
		parProdB[i, 2] = y[i, 1];
		parProdA[i, 5] = x[i, 1];
		parProdB[i, 5] = y[i, 0];
	}

	parProdRes = parProdA * parProdB;
	result = parProdRes[:, 0 : 3] - parProdRes[:, 3 : 6];
	return result;
}



template <domain D : additive3pp>
D float32[[2]] crossProduct (D float32[[2]] x, D float32[[2]] y) {

	uint[[1]] xShape = shape (x);
	uint[[1]] yShape = shape (y);

	assert(xShape[1] == 3 && yShape[1] == 3 && xShape[0] == yShape[0]);
	D float32[[2]] result (xShape[0], xShape[1]);

	D float32[[2]] parProdA (xShape[0], xShape[1] * 2), parProdB (xShape[0], xShape[1] * 2), parProdRes (xShape[0], xShape[1] * 2);

	for (uint i = 0; i < xShape[0]; i++){
		parProdA[i, 0] = x[i, 1];
		parProdB[i, 0] = y[i, 2];
		parProdA[i, 3] = x[i, 2];
		parProdB[i, 3] = y[i, 1];
		parProdA[i, 1] = x[i, 2];
		parProdB[i, 1] = y[i, 0];
		parProdA[i, 4] = x[i, 0];
		parProdB[i, 4] = y[i, 2];
		parProdA[i, 2] = x[i, 0];
		parProdB[i, 2] = y[i, 1];
		parProdA[i, 5] = x[i, 1];
		parProdB[i, 5] = y[i, 0];
	}

	parProdRes = parProdA * parProdB;
	result = parProdRes[:, 0 : 3] - parProdRes[:, 3 : 6];
	return result;
}


template <domain D : additive3pp>
D float64[[2]] crossProduct (D float64[[2]] x, D float64[[2]] y) {

	uint[[1]] xShape = shape (x);
	uint[[1]] yShape = shape (y);

	assert(xShape[1] == 3 && yShape[1] == 3 && xShape[0] == yShape[0]);
	D float64[[2]] result (xShape[0], xShape[1]);

	D float64[[2]] parProdA (xShape[0], xShape[1] * 2), parProdB (xShape[0], xShape[1] * 2), parProdRes (xShape[0], xShape[1] * 2);

	for (uint i = 0; i < xShape[0]; i++){
		parProdA[i, 0] = x[i, 1];
		parProdB[i, 0] = y[i, 2];
		parProdA[i, 3] = x[i, 2];
		parProdB[i, 3] = y[i, 1];
		parProdA[i, 1] = x[i, 2];
		parProdB[i, 1] = y[i, 0];
		parProdA[i, 4] = x[i, 0];
		parProdB[i, 4] = y[i, 2];
		parProdA[i, 2] = x[i, 0];
		parProdB[i, 2] = y[i, 1];
		parProdA[i, 5] = x[i, 1];
		parProdB[i, 5] = y[i, 0];
	}

	parProdRes = parProdA * parProdB;
	result = parProdRes[:, 0 : 3] - parProdRes[:, 3 : 6];
	return result;
}


/** @}*/
/** @}*/


/*****************************************************
	matrixMultiplication, diagMatrixMultiplication
*****************************************************/

/** \addtogroup <a3p_matrixmultiplication>
 *  @{
 *  @brief Function for multiplying two matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 */

/** \addtogroup <a3p_matrixmultiplication_mat>
 *  @{
 *  @brief Function for multiplying two matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @warning no. of columns of x must equal no. of rows of y
 *  @param x,y - 2-dimensional matrices of supported type and shape
 *  @return returns the matrix of x*y
 */

template <domain D : additive3pp>
D uint8[[2]] matrixMultiplication (D uint8[[2]] x, D uint8[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D uint8[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
							   mulVec2 (xShape[0] * yShape[1] * commonDim),
							   product (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D uint8[[1]] yColumn (commonDim);
	for(uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}

	product = mulVec1 * mulVec2;

	D uint8[[2]] result (xShape[0], yShape[1]);
	D uint8[[1]] resultVec (xShape[0] * yShape[1]);

	resultVec = sum (product, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; i++){
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}

template <domain D : additive3pp>
D uint16[[2]] matrixMultiplication (D uint16[[2]] x, D uint16[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D uint16[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
							   mulVec2 (xShape[0] * yShape[1] * commonDim),
							   product (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D uint16[[1]] yColumn (commonDim);
	for(uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}

	product = mulVec1 * mulVec2;

	D uint16[[2]] result (xShape[0], yShape[1]);
	D uint16[[1]] resultVec (xShape[0] * yShape[1]);

	resultVec = sum (product, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; i++){
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}

template <domain D : additive3pp>
D uint32[[2]] matrixMultiplication (D uint32[[2]] x, D uint32[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D uint32[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
							   mulVec2 (xShape[0] * yShape[1] * commonDim),
							   product (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D uint32[[1]] yColumn (commonDim);
	for(uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}

	product = mulVec1 * mulVec2;

	D uint32[[2]] result (xShape[0], yShape[1]);
	D uint32[[1]] resultVec (xShape[0] * yShape[1]);

	resultVec = sum (product, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; i++){
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}

template <domain D : additive3pp>
D uint[[2]] matrixMultiplication (D uint[[2]] x, D uint[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D uint[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
							   mulVec2 (xShape[0] * yShape[1] * commonDim),
							   product (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D uint[[1]] yColumn (commonDim);
	for(uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}

	product = mulVec1 * mulVec2;

	D uint[[2]] result (xShape[0], yShape[1]);
	D uint[[1]] resultVec (xShape[0] * yShape[1]);

	resultVec = sum (product, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; i++){
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}

template <domain D : additive3pp>
D int8[[2]] matrixMultiplication (D int8[[2]] x, D int8[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D int8[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
							   mulVec2 (xShape[0] * yShape[1] * commonDim),
							   product (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D int8[[1]] yColumn (commonDim);
	for(uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}

	product = mulVec1 * mulVec2;

	D int8[[2]] result (xShape[0], yShape[1]);
	D int8[[1]] resultVec (xShape[0] * yShape[1]);

	resultVec = sum (product, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; i++){
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}

template <domain D : additive3pp>
D int16[[2]] matrixMultiplication (D int16[[2]] x, D int16[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D int16[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
							   mulVec2 (xShape[0] * yShape[1] * commonDim),
							   product (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D int16[[1]] yColumn (commonDim);
	for(uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}

	product = mulVec1 * mulVec2;

	D int16[[2]] result (xShape[0], yShape[1]);
	D int16[[1]] resultVec (xShape[0] * yShape[1]);

	resultVec = sum (product, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; i++){
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}

template <domain D : additive3pp>
D int32[[2]] matrixMultiplication (D int32[[2]] x, D int32[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D int32[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
							   mulVec2 (xShape[0] * yShape[1] * commonDim),
							   product (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D int32[[1]] yColumn (commonDim);
	for(uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}

	product = mulVec1 * mulVec2;

	D int32[[2]] result (xShape[0], yShape[1]);
	D int32[[1]] resultVec (xShape[0] * yShape[1]);

	resultVec = sum (product, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; i++){
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}

template <domain D : additive3pp>
D int[[2]] matrixMultiplication (D int[[2]] x, D int[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D int[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
			   mulVec2 (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D int[[1]] yColumn (commonDim);
	for (uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}
	mulVec1 = mulVec1 * mulVec2;

	D int[[2]] result (xShape[0], yShape[1]);
	D int[[1]] resultVec = sum (mulVec1, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; ++i) {
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}

template <domain D : additive3pp>
D float32[[2]] matrixMultiplication (D float32[[2]] x, D float32[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D float32[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
							   mulVec2 (xShape[0] * yShape[1] * commonDim),
							   product (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D float32[[1]] yColumn (commonDim);
	for(uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}

	product = mulVec1 * mulVec2;

	D float32[[2]] result (xShape[0], yShape[1]);
	D float32[[1]] resultVec (xShape[0] * yShape[1]);

	resultVec = sum (product, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; i++){
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}


template <domain D : additive3pp>
D float64[[2]] matrixMultiplication (D float64[[2]] x, D float64[[2]] y) {

	// For parallelisation

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D float64[[1]] mulVec1 (xShape[0] * yShape[1] * commonDim),
							   mulVec2 (xShape[0] * yShape[1] * commonDim),
							   product (xShape[0] * yShape[1] * commonDim);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D float64[[1]] yColumn (commonDim);
	for(uint i = 0; i < yShape[1]; i++) {
		yColumn = y[:, i];
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = x[j, :];
			mulVec2[((xShape[0] * i + j) * commonDim) : ((xShape[0] * i + j + 1) * commonDim)] = yColumn;
		}
	}

	product = mulVec1 * mulVec2;

	D float64[[2]] result (xShape[0], yShape[1]);
	D float64[[1]] resultVec (xShape[0] * yShape[1]);

	resultVec = sum (product, (xShape[0] * yShape[1]));

	for (uint i = 0; i < yShape[1]; i++){
		result[:, i] = resultVec [i * xShape[0] : (i + 1) * xShape[0]];
	}

	return result;
}













/** @}*/
/** \addtogroup <a3p_matrixmultiplication_cube>
 *  @{
 *  @brief Function for multiplying two matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @warning no. of columns of x must equal no. of rows of y. Also, there should be an equal no. of matrices in both structures
 *  @param x,y - 3-dimensional matrices of supported type and shape
 *  @return We multiply across the last two dimensions and return a vector of product matrices
 */

template <domain D : additive3pp>
D uint8[[3]] matrixMultiplication (D uint8[[3]] x, D uint8[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D uint8[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D uint8[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D uint8[[3]] result (count, xShape[1], yShape[2]);
	D uint8[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}
template <domain D : additive3pp>
D uint16[[3]] matrixMultiplication (D uint16[[3]] x, D uint16[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D uint16[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D uint16[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D uint16[[3]] result (count, xShape[1], yShape[2]);
	D uint16[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}
template <domain D : additive3pp>
D uint32[[3]] matrixMultiplication (D uint32[[3]] x, D uint32[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D uint32[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D uint32[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D uint32[[3]] result (count, xShape[1], yShape[2]);
	D uint32[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}
template <domain D : additive3pp>
D uint[[3]] matrixMultiplication (D uint[[3]] x, D uint[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D uint[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D uint[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D uint[[3]] result (count, xShape[1], yShape[2]);
	D uint[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}


template <domain D : additive3pp>
D int8[[3]] matrixMultiplication (D int8[[3]] x, D int8[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D int8[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D int8[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D int8[[3]] result (count, xShape[1], yShape[2]);
	D int8[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}
template <domain D : additive3pp>
D int16[[3]] matrixMultiplication (D int16[[3]] x, D int16[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D int16[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D int16[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D int16[[3]] result (count, xShape[1], yShape[2]);
	D int16[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}
template <domain D : additive3pp>
D int32[[3]] matrixMultiplication (D int32[[3]] x, D int32[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D int32[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D int32[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D int32[[3]] result (count, xShape[1], yShape[2]);
	D int32[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}
template <domain D : additive3pp>
D int[[3]] matrixMultiplication (D int[[3]] x, D int[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D int[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D int[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D int[[3]] result (count, xShape[1], yShape[2]);
	D int[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}

template <domain D : additive3pp>
D float32[[3]] matrixMultiplication (D float32[[3]] x, D float32[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D float32[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D float32[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D float32[[3]] result (count, xShape[1], yShape[2]);
	D float32[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}
template <domain D : additive3pp>
D float64[[3]] matrixMultiplication (D float64[[3]] x, D float64[[3]] y) {
	// We multiply across the last two dimensions
	// And return a vector of product matrices

	// For parallelisation
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D float64[[1]] mulVec1 (matrixSize * commonDim * count),
					  mulVec2 (matrixSize * commonDim * count),
					  product (matrixSize * commonDim * count);

	// At the moment our matrices are kept in memory in row major order
	// We only take the column we need from memory once
	// This is also why our cycles run first over y and then over x
	D float64[[1]] yColumn (commonDim * count);


	for(uint m = 0; m < count; ++m) {
		for(uint i = 0; i < yShape[2]; ++i) {
			yColumn = y[m, :, i];
			for(uint j = 0; j < xShape[1]; ++j) {
				mulVec1[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = x[m, j, :];
				mulVec2[((xShape[1] * i + j + m * matrixSize) * commonDim) : ((xShape[1] * i + m * matrixSize + j + 1) * commonDim)] = yColumn;
			}
		}
	}

	product = mulVec1 * mulVec2;

	D float64[[3]] result (count, xShape[1], yShape[2]);
	D float64[[1]] resultVec (count * matrixSize);

	resultVec = sum (product, (matrixSize * count));

	for (uint m = 0; m < count; m++){
		for (uint i = 0; i < yShape[2]; i++){
			result[m, :, i] = resultVec [i * xShape[1] + m * matrixSize : (i + 1) * xShape[1] + m * matrixSize];
		}
	}

	return result;
}

/** @}*/

/** @}*/













/** \addtogroup <a3p_diag_matrixmultiplication>
 *  @{
 *  @brief Function for multiplying two diagonal matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 */

/** \addtogroup <a3p_diag_matrixmultiplication_mat>
 *  @{
 *  @brief Function for multiplying two diagonal matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @warning NB! This matrix multiplication is very conditional. Before using, make sure that your matrices are in the right format. **y** must be diagonal
 *  @param x,y - 2-dimensional matrices of supported type and shape
 *  @return returns the matrix of x*y
 */



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D uint8[[2]] diagMatrixMultiplication (D uint8[[2]] x, D uint8[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D uint8[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D uint8[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D uint16[[2]] diagMatrixMultiplication (D uint16[[2]] x, D uint16[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D uint16[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D uint16[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D uint32[[2]] diagMatrixMultiplication (D uint32[[2]] x, D uint32[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D uint32[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D uint32[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D uint[[2]] diagMatrixMultiplication (D uint[[2]] x, D uint[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D uint[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D uint[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D int8[[2]] diagMatrixMultiplication (D int8[[2]] x, D int8[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D int8[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D int8[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D int16[[2]] diagMatrixMultiplication (D int16[[2]] x, D int16[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D int16[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D int16[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D int32[[2]] diagMatrixMultiplication (D int32[[2]] x, D int32[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D int32[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D int32[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D int[[2]] diagMatrixMultiplication (D int[[2]] x, D int[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D int[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D int[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D float32[[2]] diagMatrixMultiplication (D float32[[2]] x, D float32[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D float32[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D float32[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D float64[[2]] diagMatrixMultiplication (D float64[[2]] x, D float64[[2]] y) {

	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[0] == yShape[1]);

	// no. of columns of x must equal no. of rows of y
	assert (xShape[1] == yShape[0]);

	uint commonDim = xShape[1];

	D float64[[1]] mulVec1 (xShape[0] * yShape[1]),
						mulVec2 (xShape[0] * yShape[1]),
						product (xShape[0] * yShape[1]);

	// We only use the elements on the diagonal
	// Also, note that yShape[1] == commonDim
	for(uint i = 0; i < yShape[1]; i++) {
		for(uint j = 0; j < xShape[0]; j++) {
			mulVec1[i + j * yShape[1]] = x[j, i];
			mulVec2[i + j * yShape[1]] = y[i, i];
		}
	}

	product = mulVec1 * mulVec2;

	D float64[[2]] result = reshape (product, xShape[0], yShape[1]);

	return result;
}












/** @}*/
/** \addtogroup <a3p_diag_matrixmultiplication_cube>
 *  @{
 *  @brief Function for multiplying two diagonal matrices
 *  @note **D** - additive3pp protection domain
 *  @note Supported types - \ref uint8 "uint8" / \ref uint16 "uint16" / \ref uint32 "uint32" / \ref uint64 "uint" / \ref int8 "int8" / \ref int16 "int16" / \ref int32 "int32" / \ref int64 "int" / \ref float32 "float32" / \ref float64 "float64"
 *  @warning NB! This matrix multiplication is very conditional. Before using, make sure that your matrices are in the right format. **y** must be diagonal
 *  @param x,y - 3-dimensional matrices of supported type and shape
 *  @return We multiply across the last two dimensions and return a vector of product matrices
 */


// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D uint8[[3]] diagMatrixMultiplication (D uint8[[3]] x, D uint8[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D uint8[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D uint8[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D uint16[[3]] diagMatrixMultiplication (D uint16[[3]] x, D uint16[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D uint16[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D uint16[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D uint32[[3]] diagMatrixMultiplication (D uint32[[3]] x, D uint32[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D uint32[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D uint32[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D uint[[3]] diagMatrixMultiplication (D uint[[3]] x, D uint[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D uint[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D uint[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D int8[[3]] diagMatrixMultiplication (D int8[[3]] x, D int8[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D int8[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D int8[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D int16[[3]] diagMatrixMultiplication (D int16[[3]] x, D int16[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D int16[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D int16[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D int32[[3]] diagMatrixMultiplication (D int32[[3]] x, D int32[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D int32[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D int32[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D int[[3]] diagMatrixMultiplication (D int[[3]] x, D int[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D int[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D int[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D float32[[3]] diagMatrixMultiplication (D float32[[3]] x, D float32[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D float32[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D float32[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}



// NB! This matrix multiplication is very conditional
// Before using, make sure that your matrices are in the right format
// y must be diagonal
template <domain D : additive3pp>
D float64[[3]] diagMatrixMultiplication (D float64[[3]] x, D float64[[3]] y) {

	// We multiply across the first two dimensions
	// And return a vector of product matrices
	uint [[1]] xShape = shape (x);
	uint [[1]] yShape = shape (y);

	// We assume that matrix y is diagonal, therefore y must be square
	assert (yShape[1] == yShape[2]);

	// no. of columns of x must equal no. of rows of y
	// Also, there should be an equal no. of matrices in both structures
	assert (xShape[2] == yShape[1] && xShape[0] == yShape[0]);

	uint commonDim = xShape[2];
	uint count = xShape[0];
	uint matrixSize = xShape[1] * yShape[2];

	D float64[[1]] mulVec1 (xShape[1] * yShape[2] * count),
						mulVec2 (xShape[1] * yShape[2] * count),
						product (xShape[1] * yShape[2] * count);

	// We only use the elements on the diagonal
	// Also, note that yShape[2] == commonDim
	for(uint m = 0; m < count; m++) {
		for(uint i = 0; i < yShape[2]; i++) {
			for(uint j = 0; j < xShape[1]; j++) {
				mulVec1[i + j * yShape[2] + m * matrixSize] = x[m, j, i];
				mulVec2[i + j * yShape[2] + m * matrixSize] = y[m, i, i];
			}
		}
	}

	product = mulVec1 * mulVec2;

	D float64[[3]] result = reshape (product, count, xShape[1], yShape[2]);

	return result;
}
/** @}*/
/** @}*/
/** @}*/