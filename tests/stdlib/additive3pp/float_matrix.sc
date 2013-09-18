/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module matrix_test;

import stdlib;
import matrix;
import additive3pp;
import a3p_matrix;
import oblivious;
import a3p_random;
import a3p_sort;
import a3p_bloom;
import x3p_string;
import x3p_aes;
import x3p_join;
import profiling;
import test_utility;

domain pd_a3p additive3pp;

public uint all_tests;
public uint succeeded_tests;


template<type T>
T random_float(T data){
    T rand = 1;
    pd_a3p uint32 temp;
    pd_a3p int8 temp2;
    T scalar;
    T scalar2;
    for(uint i = 0; i < 2; ++i){   
        scalar = 0;
        while(scalar == 0 || scalar2 == 0){
            scalar = (T) declassify(randomize(temp));
            scalar2 = (T) declassify(randomize(temp2));
        }
        if((i % 2) == 0){
            rand *= scalar;
            rand *= scalar2;
        }
        else{
            rand /= scalar;
            rand /= scalar2;
        }
    }
    return rand;
}

template<domain D:additive3pp,type T>
D T[[1]] random(D T[[1]] data){
    uint x_shape = size(data);
    T[[1]] rand (x_shape) = 1;
    pd_a3p uint32[[1]] temp (x_shape);
    pd_a3p int8[[1]] temp2 (x_shape);
    T[[1]] scalar (x_shape);
    T[[1]] scalar2 (x_shape);
    for(uint i = 0; i < 2; ++i){   
        scalar[0] = 0;
        while(any(scalar == 0) || any(scalar2 == 0)){
            scalar = (T) declassify(randomize(temp));
            scalar2 = (T) declassify(randomize(temp2));
        }
        if((i % 2) == 0){
            rand *= scalar;
            rand *= scalar2;
        }
        else{
            rand /= scalar;
            rand /= scalar2;
        }
    }
    pd_a3p T[[1]] result = rand;
    return result;
}

template<domain D:additive3pp,type T>
D T[[2]] random(D T[[2]] data){
    uint x_shape = shape(data)[0];
    uint y_shape = shape(data)[1];
    T[[2]] rand (x_shape,y_shape) = 1;
    pd_a3p uint32[[2]] temp (x_shape,y_shape);
    pd_a3p int8[[2]] temp2 (x_shape,y_shape);
    T[[2]] scalar (x_shape,y_shape);
    T[[2]] scalar2 (x_shape,y_shape);
    for(uint i = 0; i < 2; ++i){   
        scalar[0,0] = 0;
        while(any(scalar == 0) || any(scalar2 == 0)){
            scalar = (T) declassify(randomize(temp));
            scalar2 = (T) declassify(randomize(temp2));
        }
        if((i % 2) == 0){
            rand *= scalar;
            rand *= scalar2;
        }
        else{
            rand /= scalar;
            rand /= scalar2;
        }
    }
    pd_a3p T[[2]] result = rand;
    return result;
}

template<domain D:additive3pp,type T>
D T[[3]] random(D T[[3]] data){
    uint x_shape = shape(data)[0];
    uint y_shape = shape(data)[1];
    uint z_shape = shape(data)[2];
    T[[3]] rand (x_shape,y_shape,z_shape) = 1;
    pd_a3p uint32[[3]] temp (x_shape,y_shape,z_shape);
    pd_a3p int8[[3]] temp2 (x_shape,y_shape,z_shape);
    T[[3]] scalar (x_shape,y_shape,z_shape);
    T[[3]] scalar2 (x_shape,y_shape,z_shape);
    for(uint i = 0; i < 2; ++i){   
        scalar[0,0,0] = 0;
        while(any(scalar == 0) || any(scalar2 == 0)){
            scalar = (T) declassify(randomize(temp));
            scalar2 = (T) declassify(randomize(temp2));
        }
        if((i % 2) == 0){
            rand *= scalar;
            rand *= scalar2;
        }
        else{
            rand /= scalar;
            rand /= scalar2;
        }
    }
    pd_a3p T[[3]] result = rand;
    return result;
}

template<domain D : additive3pp,type T>
void test_transpose(D T data){
	{
		pd_a3p T[[2]] mat (0,0);
		mat = transpose(mat);
		pd_a3p T[[2]] mat2 (2,0);
		mat2 = transpose(mat2);
		pd_a3p T[[2]] mat3 (0,2);
		mat3 = transpose(mat3);
	}
	bool result = true;
	pd_a3p T[[2]] mat (3,3);
	mat = random(mat);
	pd_a3p T[[2]] mat2 = transpose(mat);
	for(uint i = 0; i < 3; ++i){
		if(!all(declassify(mat[:,i]) == declassify(mat2[i,:]))){
			result = false;
		}	
	}
	if(result){
 		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
 	}
 	else{
 		print("FAILURE! transposing failed"); print("Got this : ");
 		printMatrix(declassify(mat2)); print(" From this: ");
 		printMatrix(declassify(mat));
 		all_tests = all_tests +1;
 	}
}

template<domain D : additive3pp,type T>
void test_transpose2(D T data){
	{
		pd_a3p T[[3]] mat (0,0,0);
		mat = transpose(mat);
		pd_a3p T[[3]] mat2 (0,2,0);
		mat2 = transpose(mat2);
		pd_a3p T[[3]] mat3 (0,0,2);
		mat3 = transpose(mat3);
		pd_a3p T[[3]] mat4 (2,0,0);
		mat4 = transpose(mat4);
		pd_a3p T[[3]] mat5 (2,0,2);
		mat5 = transpose(mat5);
		pd_a3p T[[3]] mat6 (2,2,0);
		mat6 = transpose(mat6);
	}
	bool result = true;
	pd_a3p T[[3]] mat (2,2,2);
	mat = random(mat);
	pd_a3p T[[3]] mat2 = transpose(mat);
	for(uint i = 0; i < 2; ++i){
		if(!all(declassify(mat[:,i,:]) == declassify(mat2[:,:,i]))){
			result = false;
		}
	}
	if(result){
 		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
 	}
 	else{
 		print("FAILURE! transposing failed"); print("Got this : ");
 		printVector(flatten(declassify(mat2))); print(" From this: ");
 		printVector(flatten(declassify(mat)));
 		all_tests = all_tests +1;
 	}
}

template<type T>
void row_sum_test(T data){
	{
		T[[2]] mat (3,0);
		T[[1]] result = rowSums(mat);
	}
	pd_a3p T[[2]] mat (3,3);
	T[[1]] control (3);
	mat = random(mat);
	T[[1]] row_sums = rowSums(declassify(mat));
	for(uint i = 0; i < 3;++i){
		control[i] = sum(declassify(mat[i,:]));
	}
	if(!all(row_sums == control)){
		print("FAILURE! summing rows failed"); print("Got this : ");
 		printVector(row_sums); print(" From this: ");
 		printVector(control);
 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<type T>
void dot_product(T data){
	{
		T[[1]] vec (0);
		T[[1]] vec2 (0);
		T scalar = dotProduct(vec,vec2);
	}
	pd_a3p T[[1]] vec (6);
	pd_a3p T[[1]] vec2 (6);
	vec = random(vec); vec2 = random(vec2);
	T scalar = dotProduct(declassify(vec),declassify(vec2));
	T control = sum(declassify(vec) * declassify(vec2));
	if(scalar == control){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! dot product wrong"); print("Got this : ");
 		print(scalar); print(" From these: "); print("vector 1: ");
 		printVector(declassify(vec)); print("vector 2: ");
 		printVector(declassify(vec2));
 		all_tests = all_tests +1;
	}
}

template<type T>
void dot_product_matrix(T data){
	{
		T[[2]] mat (3,0);
		T[[2]] mat2 (3,0);
		T[[1]] result = dotProduct(mat,mat2);
	}
	pd_a3p T[[2]] mat (3,3);
	pd_a3p T[[2]] mat2 (3,3);
	mat = random(mat); mat2 = random(mat2);
	T[[1]] vec = dotProduct(declassify(mat),declassify(mat2));
	T[[1]] control = rowSums(declassify(mat) * declassify(mat2));
	if(all(vec == control)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! dot product wrong"); print("Got this : ");
 		printVector(vec); print(" From these: "); print("Matrix 1: ");
 		printMatrix(declassify(mat)); print("Matrix 2: ");
 		printMatrix(declassify(mat2));
 		all_tests = all_tests +1;
	}
}

template<type T>
void cross_product(T data){
	pd_a3p T[[1]] vec (3);
	pd_a3p T[[1]] vec2 (3);
	vec = random(vec); vec2 = random(vec2);
	T[[1]] x = declassify(vec);
	T[[1]] y = declassify(vec2);
	T[[1]] vec3 = crossProduct(declassify(vec),declassify(vec2));
	T[[1]] control (3);
	control[0] = x[1] * y[2] - x[2] * y[1];
	control[1] = x[2] * y[0] - x[0] * y[2];
	control[2] = x[0] * y[1] - x[1] * y[0];
	if(all(vec3 == control)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! cross product wrong"); print("Got this : ");
 		printVector(vec3); print(" From these: "); print("vector 1: ");
 		printVector(x); print("vector 2: ");
 		printVector(y); print("But expected this: ");
 		printVector(control);
 		all_tests = all_tests +1;
	}
}

template<type T>
void cross_product_matrix(T data){
	{
		T[[2]] mat (0,3);
		T[[2]] mat2 (0,3);
		T[[2]] result = crossProduct(mat,mat2);
	}
	pd_a3p T[[2]] mat (3,3);
	pd_a3p T[[2]] mat2 (3,3);
	mat = random(mat); mat2 = random(mat2);
	T[[2]] x = declassify(mat);
	T[[2]] y = declassify(mat2);
	T[[2]] mat3 = crossProduct(declassify(mat),declassify(mat2));
	T[[2]] control (3,3);
	
	for (uint i = 0; i < 3; ++i){
		control[i,:] = crossProduct(x[i,:], y[i,:]);		
	}

	if(all(mat3 == control)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! cross product wrong"); print("Got this : ");
 		printMatrix(mat3); print(" From these: "); print("matrix 1: ");
 		printMatrix(x); print("matrix 2: ");
 		printMatrix(y); print("But expected this: ");
 		printMatrix(control);
 		all_tests = all_tests +1;
	}
}

template<type T>
void mat_multiplication(T data){
	{
		T[[2]] mat (3,0);
		T[[2]] mat2 (0,3);
		T[[2]] result = matrixMultiplication(mat,mat2);
	}
	pd_a3p T[[2]] mat (3,4);
	pd_a3p T[[2]] mat2 (4,3);
	mat = random(mat); mat2 = random(mat2);
	T[[2]] mat3 (3,3) = matrixMultiplication(declassify(mat),declassify(mat2));
	T[[2]] control (3,3);
	for(uint i = 0; i < 3;++i){
		for(uint j = 0; j < 3;++j){
			control[i,j] = sum(declassify(mat[i,:]) * declassify(mat2[:,j]));
		}
	}
	if(all(mat3 == control)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! multiplication failed"); print("Got this : ");
 		printMatrix(mat3); print(" From these: "); print("matrix 1: ");
 		printMatrix(declassify(mat)); print("matrix 2: ");
 		printMatrix(declassify(mat2)); print("But expected this: ");
 		printMatrix(control);
 		all_tests = all_tests +1;
		}
}
//---------------------------------------------
// a3p_matrix templates
//---------------------------------------------
template<type T>
void a3p_row_sum_test(T data){
	{
		pd_a3p T[[2]] mat2 (3,0);
		pd_a3p T[[1]] result = rowSums(mat2);
	}
	pd_a3p T[[2]] mat (3,3);
	T[[1]] control (3);
	mat = random(mat);
	pd_a3p T[[1]] row_sums = rowSums(mat);
	for(uint i = 0; i < 3;++i){
		control[i] = sum(declassify(mat[i,:]));
	}
	if(!all((declassify(row_sums) / control) >= 0.99) && !all((declassify(row_sums) / control) <= 1.01)){
		print("FAILURE! summing rows failed"); print("Got this : ");
 		printVector(declassify(row_sums)); print(" From this: ");
 		printVector(control);
 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<type T>
void a3p_dot_product(T data){
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p T[[1]] vec2 (0);
		pd_a3p T scalar = dotProduct(vec,vec2);
	}
	pd_a3p T[[1]] vec (6);
	pd_a3p T[[1]] vec2 (6);
	vec = random(vec); vec2 = random(vec2);
	pd_a3p T scalar = dotProduct(vec,vec2);
	T control = sum(declassify(vec) * declassify(vec2));
	if((declassify(scalar) / control) >= 0.99 && (declassify(scalar) / control) <= 1.01){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! dot product wrong"); print("Got this : ");
 		print(declassify(scalar)); print(" From these: "); print("vector 1: ");
 		printVector(declassify(vec)); print("vector 2: ");
 		printVector(declassify(vec2));
 		all_tests = all_tests +1;
	}
}

template<type T>
void a3p_dot_product_matrix(T data){
	{
		pd_a3p T[[2]] mat (3,0);
		pd_a3p T[[2]] mat2 (3,0);
		pd_a3p T[[1]] result = dotProduct(mat,mat2);
	}
	pd_a3p T[[2]] mat (3,3);
	pd_a3p T[[2]] mat2 (3,3);
	mat = random(mat); mat2 = random(mat2);
	pd_a3p T[[1]] vec = dotProduct(mat,mat2);
	T[[1]] control = rowSums(declassify(mat) * declassify(mat2));
	if(all((declassify(vec) / control) >= 0.99) && all((declassify(vec) / control) <= 1.01)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! dot product wrong"); print("Got this : ");
 		printVector(declassify(vec)); print(" From these: "); print("Matrix 1: ");
 		printMatrix(declassify(mat)); print("Matrix 2: ");
 		printMatrix(declassify(mat2));
 		all_tests = all_tests +1;
	}
}

template<type T>
void a3p_cross_product(T data){
	{
		pd_a3p T[[2]] mat (0,3);
		pd_a3p T[[2]] mat2 (0,3);
		pd_a3p T[[2]] result = crossProduct(mat,mat2);
	}
	pd_a3p T[[1]] vec (3);
	pd_a3p T[[1]] vec2 (3);
	vec = random(vec); vec2 = random(vec2);
	T[[1]] x = declassify(vec);
	T[[1]] y = declassify(vec2);
	pd_a3p T[[1]] vec3 = crossProduct(vec,vec2);
	T[[1]] control (3);
	control[0] = x[1] * y[2] - x[2] * y[1];
	control[1] = x[2] * y[0] - x[0] * y[2];
	control[2] = x[0] * y[1] - x[1] * y[0];
	if(all((declassify(vec3) / control) >= 0.99) && all((declassify(vec3) / control) <= 1.01)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! cross product wrong"); print("Got this : ");
 		printVector(declassify(vec3)); print(" From these: "); print("vector 1: ");
 		printVector(x); print("vector 2: ");
 		printVector(y); print("But expected this: ");
 		printVector(control);
 		all_tests = all_tests +1;
	}
}

template<type T>
void a3p_cross_product_matrix(T data){
	{
		pd_a3p T[[2]] mat (3,0);
		pd_a3p T[[2]] mat2 (0,3);
		pd_a3p T[[2]] result = matrixMultiplication(mat,mat2);
	}
	pd_a3p T[[2]] mat (3,3);
	pd_a3p T[[2]] mat2 (3,3);
	mat = random(mat); mat2 = random(mat2);
	T[[2]] x = declassify(mat);
	T[[2]] y = declassify(mat2);
	pd_a3p T[[2]] mat3 = crossProduct(mat,mat2);
	T[[2]] control (3,3);
	
	for (uint i = 0; i < 3; ++i){
		control[i,:] = crossProduct(x[i,:], y[i,:]);		
	}

	if(all((declassify(mat3) / control) >= 0.99) && all((declassify(mat3) / control) <= 1.01) ){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! cross product wrong"); print("Got this : ");
 		printMatrix(declassify(mat3)); print(" From these: "); print("matrix 1: ");
 		printMatrix(x); print("matrix 2: ");
 		printMatrix(y); print("But expected this: ");
 		printMatrix(control);
 		all_tests = all_tests +1;
	}
}

template<type T>
void a3p_mat_multiplication(T data){
	{
		pd_a3p T[[2]] mat (0,0);
		pd_a3p T[[2]] mat2 (0,0);
		pd_a3p T[[2]] result = diagMatrixMultiplication(mat,mat2);
		pd_a3p T[[2]] mat3 (3,0);
		pd_a3p T[[2]] mat4 (0,0);
		result = diagMatrixMultiplication(mat3,mat4);
	}
	pd_a3p T[[2]] mat (3,4);
	pd_a3p T[[2]] mat2 (4,3);
	mat = random(mat); mat2 = random(mat2);
	pd_a3p T[[2]] mat3 (3,3) = matrixMultiplication(mat,mat2);
	T[[2]] control (3,3);
	T[[2]] matb = declassify(mat);
	T[[2]] mat2b = declassify(mat2);
	for(uint i = 0; i < shape(declassify(mat3))[0];++i){
		for(uint j = 0; j < shape(declassify(mat3))[1];++j){
			control[i,j] = sum(matb[i,:] * mat2b[:,j]);
		}
	}
	if(all((declassify(mat3) / control) >= 0.99) && all((declassify(mat3) / control) <= 1.01) ){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! multiplication failed"); print("Got this : ");
 		printMatrix(declassify(mat3)); print(" From these: "); print("matrix 1: ");
 		printMatrix(declassify(mat)); print("matrix 2: ");
 		printMatrix(declassify(mat2)); print("But expected this: ");
 		printMatrix(control);
 		all_tests = all_tests +1;
		}
}

template<type T>
void a3p_diag_mat_multiplication(T data){
	{
		pd_a3p T[[2]] mat (0,0);
		pd_a3p T[[2]] mat2 (0,0);
		pd_a3p T[[2]] result = diagMatrixMultiplication(mat,mat2);
		pd_a3p T[[2]] mat3 (3,0);
		pd_a3p T[[2]] mat4 (0,0);
		result = diagMatrixMultiplication(mat3,mat4);
	}
	pd_a3p T[[2]] mat (3,4);
	pd_a3p T[[2]] mat2 (4,4);
	mat = random(mat); mat2 = random(mat2);
	for(uint i = 0; i < shape(declassify(mat2))[0]; ++i){
        for(uint j = 0; j < shape(declassify(mat2))[1]; ++j){
            if(i != j){
                mat2[i,j] = 0;
            }
        }
    }
    T[[2]] matb = declassify(mat);
	T[[2]] mat2b = declassify(mat2);
    pd_a3p T[[2]] mat3 (3,4) = diagMatrixMultiplication(mat,mat2);
    T[[2]] control (3,4);
    for(uint i = 0; i < 3;++i){
    	for(uint j = 0; j < 4;++j){
    		control[i,j] = matb[i,j] * mat2b[j,j];
    	}

    }
    if(all((declassify(mat3) / control) >= 0.99) && all((declassify(mat3) / control) <= 1.01) ){
    	succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
    }
    else{
    	print("FAILURE! diagonal multiplication failed"); print("Got this : ");
 		printMatrix(declassify(mat3)); print(" From these: "); print("matrix 1: ");
 		printMatrix(declassify(mat)); print("matrix 2: ");
 		printMatrix(declassify(mat2)); print("But expected this: ");
 		printMatrix(control);
 		all_tests = all_tests +1;
    }
}

template<type T>
void a3p_mat3D_multiplication(T data){
	pd_a3p T[[3]] mat (2,2,2);
	pd_a3p T[[3]] mat2 (2,2,2);
	mat = random(mat); mat2 = random(mat2);
	pd_a3p T[[3]] mat3 (2,2,2) = matrixMultiplication(mat,mat2);
	T[[3]] control (2,2,2);
	T[[3]] matb = declassify(mat);
	T[[3]] mat2b = declassify(mat2);
	for(uint h = 0; h < shape(mat3)[0];++h){
		for(uint i = 0; i < shape(declassify(mat3))[1];++i){
			for(uint j = 0; j < shape(declassify(mat3))[2];++j){
				control[h,i,j] = sum(matb[h,i,:] * mat2b[h,:,j]);
			}
		}
	}
	if(all((declassify(mat3) / control) >= 0.99) && all((declassify(mat3) / control) <= 1.01) ){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! multiplication failed"); print("Got this : ");
 		print3dArray(declassify(mat3)); print(" From these: "); print("matrix 1: ");
 		print3dArray(declassify(mat)); print("matrix 2: ");
 		print3dArray(declassify(mat2)); print("But expected this: ");
 		print3dArray(control);
 		all_tests = all_tests +1;
	}
}


template<type T>
void a3p_diag3D_mat_multiplication(T data){
	{
		pd_a3p T[[3]] mat (0,0,0);
		pd_a3p T[[3]] mat2 (0,0,0);
		pd_a3p T[[3]] result = diagMatrixMultiplication(mat,mat2);
		pd_a3p T[[3]] mat3 (3,0,0);
		pd_a3p T[[3]] mat4 (3,0,0);
		result = diagMatrixMultiplication(mat3,mat4);
		pd_a3p T[[3]] mat5 (0,0,3);
		pd_a3p T[[3]] mat6 (0,3,3);
		result = diagMatrixMultiplication(mat5,mat6);
	}
	pd_a3p T[[3]] mat (2,2,2);
	pd_a3p T[[3]] mat2 (2,2,2);
	mat = random(mat); mat2 = random(mat2);
	for(uint h = 0; h < shape(declassify(mat2))[0];++h){
		for(uint i = 0; i < shape(declassify(mat2))[1]; ++i){
    	    for(uint j = 0; j < shape(declassify(mat2))[2]; ++j){
        	    if(i != j){
            	    mat2[h,i,j] = 0;
            	}
        	}
    	}
    }
    pd_a3p T[[3]] mat3 (2,2,2) = diagMatrixMultiplication(mat,mat2);
    T[[3]] control (2,2,2);
    T[[3]] matb = declassify(mat);
	T[[3]] mat2b = declassify(mat2);
    for(uint h = 0; h < shape(declassify(mat))[0];++h){
    	for(uint i = 0; i < shape(declassify(mat))[1];++i){
    		for(uint j = 0; j < shape(declassify(mat))[2];++j){
    			control[h,i,j] = matb[h,i,j] * mat2b[h,j,j];
    		}
    	}
    }
    if(all((declassify(mat3) / control) >= 0.99) && all((declassify(mat3) / control) <= 1.01) ){
    	succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
    }
    else{
    	print("FAILURE! diagonal multiplication failed"); print("Got this : ");
 		print3dArray(declassify(mat3)); print(" From these: "); print("matrix 1: ");
 		print3dArray(declassify(mat)); print("matrix 2: ");
 		print3dArray(declassify(mat2)); print("But expected this: ");
 		print3dArray(control);
 		all_tests = all_tests +1;
    }
}

template<type T>
void a3p_vec_length(T data){
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p T length = vecLength(vec);
	}
	pd_a3p T[[1]] vec (3);
	vec = random(vec);
	T[[1]] vec2 = declassify(vec);
	T length = declassify(vecLength(vec));
	pd_a3p T temp = (vec2[0]*vec2[0]) + (vec2[1]*vec2[1]) + (vec2[2]*vec2[2]);
	T control = declassify(sqrt(temp));
	if((length / control) >= 0.99 && (length / control) <= 1.01){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
    }
    else{
    	all_tests = all_tests +1;
    	print("FAILURE! Expected: ",control, " but got: ", length);
    }
}

template<type T>
void a3p_unit_vec(T data){
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p T[[1]] unit_vector = unitVector(vec);
	}
	pd_a3p T[[1]] vec (3);
	vec = random(vec);
	T[[1]] vec2 = declassify(vec);
	T[[1]] unit_vec = declassify(unitVector(vec));
	pd_a3p T temp2 = (vec2[0]*vec2[0]) + (vec2[1]*vec2[1]) + (vec2[2]*vec2[2]);
	T temp = declassify(sqrt(temp2));
	T[[1]] control = (vec2 / temp);
	if(all((unit_vec / control) >= 0.99) && all((unit_vec / control) <= 1.01)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
    }
    else{
    	all_tests = all_tests +1;
    	print("FAILURE! Expected: "); 
    	printVector(control);
    	print("but got: ");
    	printVector(unit_vec);
    }
}

template<type T>
void a3p_vec_length2D(T data){
	{
		pd_a3p T[[2]] vec (0,0);
		pd_a3p T[[1]] length = vecLength(vec);
		pd_a3p T[[2]] vec2 (2,0);
		length = vecLength(vec2);
		pd_a3p T[[2]] vec3 (0,2);
		length = vecLength(vec3);
	}
	pd_a3p T[[2]] vec (3,3);
	vec = random(vec);
	T[[2]] vec2 = declassify(vec);
	T[[1]] unit_vec = declassify(vecLength(vec));
	pd_a3p T[[1]] temp2 (3); 
	temp2[0] = (vec2[0,0]*vec2[0,0]) + (vec2[0,1]*vec2[0,1]) + (vec2[0,2]*vec2[0,2]);
	temp2[1] = (vec2[1,0]*vec2[1,0]) + (vec2[1,1]*vec2[1,1]) + (vec2[1,2]*vec2[1,2]);
	temp2[2] = (vec2[2,0]*vec2[2,0]) + (vec2[2,1]*vec2[2,1]) + (vec2[2,2]*vec2[2,2]);
	T[[1]] control = declassify(sqrt(temp2));
	if(all((unit_vec / control) >= 0.99) && all((unit_vec / control) <= 1.01)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
    }
    else{
    	all_tests = all_tests +1;
    	print("FAILURE! Expected: "); 
    	printVector(control);
    	print("but got: ");
    	printVector(unit_vec);
    }
}

template<type T>
void a3p_unit_vec2D(T data){
	{
		pd_a3p T[[2]] vec (0,0);
		pd_a3p T[[2]] unit_vector = unitVector(vec);
		pd_a3p T[[2]] vec2 (2,0);
		unit_vector = unitVector(vec2);
		pd_a3p T[[2]] vec3 (0,2);
		unit_vector = unitVector(vec3);
	}
	pd_a3p T[[2]] vec (3,3);
	vec = random(vec);
	T[[2]] vec2 = declassify(vec);

	T[[1]] unit_vec = flatten(declassify(unitVector(vec)));

	pd_a3p T[[1]] temp3 (3); 
	temp3[0] = (vec2[0,0]*vec2[0,0]) + (vec2[0,1]*vec2[0,1]) + (vec2[0,2]*vec2[0,2]);
	temp3[1] = (vec2[1,0]*vec2[1,0]) + (vec2[1,1]*vec2[1,1]) + (vec2[1,2]*vec2[1,2]);
	temp3[2] = (vec2[2,0]*vec2[2,0]) + (vec2[2,1]*vec2[2,1]) + (vec2[2,2]*vec2[2,2]);
	T[[1]] temp2 = declassify(sqrt(temp3));
	T[[2]] temp (3,3);
	temp[0,:] = (vec2[0,:] / temp2[0]);
	temp[1,:] = (vec2[1,:] / temp2[1]);
	temp[2,:] = (vec2[2,:] / temp2[2]);
	T[[1]] control = flatten(temp);


	if(all((unit_vec / control) >= 0.99) && all((unit_vec / control) <= 1.01)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
    }
    else{
    	all_tests = all_tests +1;
    	print("FAILURE! Expected: "); 
    	printVector(control);
    	print("but got: ");
    	printVector(unit_vec);
    }
}

template<type T>
void determinant_test(T data){
	{
		T[[2]] mat (0,0);
		T result = determinant(mat);
		T[[2]] mat2 (2,0);
		result = determinant(mat2);
		T[[2]] mat3 (0,2);
		result = determinant(mat3);		
	}
	T[[2]] mat (3,3);
    T[[1]] vec1 (3) = {1,2,3};
    T[[1]] vec2 (3) = {4,5,6};
    T[[1]] vec3 (3) = {7,8,9};
    mat[0,:] = vec1;
    mat[1,:] = vec2;
    mat[2,:] = vec3;
    if((determinant(mat) + (0)) <= 0.01){
    	print("SUCCESS!");
    	all_tests += 1;
    	succeeded_tests += 1;
    }
    else{
    	print("FAILURE! determinant failed. Expected: 0 But got: ", determinant(mat));
    }
    vec1 = {3,6,2};
    vec2 = {5,1,9};
    vec3 = {5,2,1};
    mat[0,:] = vec1;
    mat[1,:] = vec2;
    mat[2,:] = vec3;
    if((determinant(mat) - (199)) <= 0.01){
    	print("SUCCESS!");
    	all_tests += 1;
    	succeeded_tests += 1;
    }
    else{
    	print("FAILURE! determinant failed. Expected: 199 But got: ", determinant(mat));
    }
    vec1 = {4.650, 6.450, 1.760};
    vec2 = {5.980, 2.540, 8.090};
    vec3 = {3.280, 7.910, 9.010};
    mat[0,:] = vec1;
    mat[1,:] = vec2;
    mat[2,:] = vec3;
    if((determinant(mat) - (-298.930)) <= 0.01){
    	print("SUCCESS!");
    	all_tests += 1;
    	succeeded_tests += 1;
    }
    else{
    	print("FAILURE! determinant failed. Expected: -298.930 But got: ", determinant(mat));
    }
}

void main(){

	print("Matrix test: start");

	print("TEST 1: Matrix transpose 2D and 3D");
	{
		print("float32");
		pd_a3p float32 data = 0.0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("float64");
		pd_a3p float64 data = 0.0;
		test_transpose(data);
		test_transpose2(data);
	}
	print("TEST 2: Matrix row sums");
	{
		print("float32");
		row_sum_test(0::float32);
	}
	{
		print("float64");
		row_sum_test(0::float64);
	}
	print("TEST 3: Dot product for vectors");
	{
		print("float32");
		dot_product(0::float32);
	}
	{
		print("float64");
		dot_product(0::float64);
	}
	print("TEST 4: Dot product for matrices");
	{
		print("float32");
		dot_product_matrix(0::float32);
	}
	{
		print("float64");
		dot_product_matrix(0::float64);
	}
	print("TEST 5: cross product for vectors");
	{
		print("float32");
		cross_product(0::float32);
	}
	{
		print("float64");
		cross_product(0::float64);
	}
	print("TEST 6: cross product for matrices");
	{
		print("float32");
		cross_product_matrix(0::float32);
	}
	{
		print("float64");
		cross_product_matrix(0::float64);
	}
	print("TEST 7: Matrix multiplication");
	{
		print("float32");
		mat_multiplication(0::float32);
	}
	{
		print("float64");
		mat_multiplication(0::float64);
	}
	print("TEST 8: a3p Matrix row sums");
	{
		print("float32");
		a3p_row_sum_test(0::float32);
	}
	{
		print("float64");
		a3p_row_sum_test(0::float64);
	}
	print("TEST 9: a3p Matrix dotproduct for vectors");
	{
		print("float32");
		a3p_dot_product(0::float32);
	}
	{
		print("float64");
		a3p_dot_product(0::float64);
	}
	print("TEST 10: a3p Matrix dotproduct for matrices");
	{
		print("float32");
		a3p_dot_product_matrix(0::float32);
	}
	{
		print("float64");
		a3p_dot_product_matrix(0::float64);
	}
	print("TEST 11: a3p cross product for vectors");
	{
		print("float32");
		a3p_cross_product(0::float32);
	}
	{
		print("float64");
		a3p_cross_product(0::float64);
	}
	print("TEST 12: a3p cross product for matrices");
	{
		print("float32");
		a3p_cross_product_matrix(0::float32);
	}
	{
		print("float64");
		a3p_cross_product_matrix(0::float64);
	}
	print("TEST 13: a3p 2D Matrix multiplication");
	{
		print("float32");
		a3p_mat_multiplication(0::float32);
	}
	{
		print("float64");
		a3p_mat_multiplication(0::float64);
	}
	print("TEST 14: a3p 2D Diagonal matrix multiplication");
	{
		print("float32");
		a3p_diag_mat_multiplication(0::float32);
	}
	{
		print("float64");
		a3p_diag_mat_multiplication(0::float64);
	}
	print("TEST 15: a3p 3D Matrix multiplication");
	{
		print("float32");
		a3p_mat3D_multiplication(0::float32);
	}
	{
		print("float64");
		a3p_mat3D_multiplication(0::float64);
	}
	print("TEST 16: a3p 3D Diagonal matrix multiplication");
	{
		print("float32");
		a3p_diag3D_mat_multiplication(0::float32);
	}
	{
		print("float64");
		a3p_diag3D_mat_multiplication(0::float64);
	}
	print("TEST 17: a3p vector length");
	{
		print("float32");
		a3p_vec_length(0::float32);
	}
	{
		print("float64");
		a3p_vec_length(0::float64);
	}
	print("TEST 18: a3p unit vector");
	{
		print("float32");
		a3p_unit_vec(0::float32);
	}
	{
		print("float64");
		a3p_unit_vec(0::float64);
	}
	print("TEST 19: a3p vector length 2D");
	{
		print("float32");
		a3p_vec_length2D(0::float32);
	}
	{
		print("float64");
		a3p_vec_length2D(0::float64);
	}
	print("TEST 20: a3p unit vector 2D");
	{
		print("float32");
		a3p_unit_vec2D(0::float32);
	}
	{
		print("float64");
		a3p_unit_vec2D(0::float64);
	}
	print("TEST 21: Determinant");
	{
		print("float32");
		determinant_test(0::float32);
	}
	{
		print("float64");
		determinant_test(0::float64);
	}




	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);

    test_report(all_tests, succeeded_tests);
}
