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

domain pd_a3p additive3pp;

public uint32 all_tests;
public uint32 succeeded_tests;

template<type T>
void row_sum_test(T data){
	{
		pd_a3p T[[2]] mat2 (3,0);
		pd_a3p T[[1]] result = rowSums(mat2);
	}
	pd_a3p T[[2]] mat (5,5);
	T[[1]] control (5);
	mat = randomize(mat);
	pd_a3p T[[1]] row_sums = rowSums(mat);
	for(uint i = 0; i < 5;++i){
		control[i] = sum(declassify(mat[i,:]));
	}
	if(!all(declassify(row_sums) == control)){
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
void dot_product(T data){
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p T[[1]] vec2 (0);
		pd_a3p T scalar = dotProduct(vec,vec2);
	}
	pd_a3p T[[1]] vec (15);
	pd_a3p T[[1]] vec2 (15);
	vec = randomize(vec); vec2 = randomize(vec2);
	pd_a3p T scalar = dotProduct(vec,vec2);
	T control = sum(declassify(vec) * declassify(vec2));
	if(declassify(scalar) == control){
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
void dot_product_matrix(T data){
	{
		pd_a3p T[[2]] mat (3,0);
		pd_a3p T[[2]] mat2 (3,0);
		pd_a3p T[[1]] result = dotProduct(mat,mat2);
	}
	pd_a3p T[[2]] mat (10,10);
	pd_a3p T[[2]] mat2 (10,10);
	mat = randomize(mat); mat2 = randomize(mat2);
	pd_a3p T[[1]] vec = dotProduct(mat,mat2);
	T[[1]] control = rowSums(declassify(mat) * declassify(mat2));
	if(all(declassify(vec) == control)){
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
void cross_product(T data){
	pd_a3p T[[1]] vec (3);
	pd_a3p T[[1]] vec2 (3);
	vec = randomize(vec); vec2 = randomize(vec2);
	T[[1]] x = declassify(vec);
	T[[1]] y = declassify(vec2);
	pd_a3p T[[1]] vec3 = crossProduct(vec,vec2);
	T[[1]] control (3);
	control[0] = x[1] * y[2] - x[2] * y[1];
	control[1] = x[2] * y[0] - x[0] * y[2];
	control[2] = x[0] * y[1] - x[1] * y[0];
	if(all(declassify(vec3) == control)){
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
void cross_product_matrix(T data){
	{
		pd_a3p T[[2]] mat (0,3);
		pd_a3p T[[2]] mat2 (0,3);
		pd_a3p T[[2]] result = crossProduct(mat,mat2);
	}
	pd_a3p T[[2]] mat (3,3);
	pd_a3p T[[2]] mat2 (3,3);
	mat = randomize(mat); mat2 = randomize(mat2);
	T[[2]] x = declassify(mat);
	T[[2]] y = declassify(mat2);
	pd_a3p T[[2]] mat3 = crossProduct(mat,mat2);
	T[[2]] control (3,3);
	
	for (uint i = 0; i < 3; ++i){
		control[i,:] = crossProduct(x[i,:], y[i,:]);		
	}

	if(all(declassify(mat3) == control)){
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
void mat_multiplication(T data){
	{
		pd_a3p T[[2]] mat (3,0);
		pd_a3p T[[2]] mat2 (0,3);
		pd_a3p T[[2]] result = matrixMultiplication(mat,mat2);
	}
	for(uint k = 3; k < 6;++k){
		for(uint l = 5; l > 2;--l){
			pd_a3p T[[2]] mat (k,l);
			pd_a3p T[[2]] mat2 (l,k);
			mat = randomize(mat); mat2 = randomize(mat2);
			pd_a3p T[[2]] mat3 (k,k) = matrixMultiplication(mat,mat2);
			T[[2]] control (k,k);
			T[[2]] matb = declassify(mat);
			T[[2]] mat2b = declassify(mat2);
			for(uint i = 0; i < shape(declassify(mat3))[0];++i){
				for(uint j = 0; j < shape(declassify(mat3))[1];++j){
					control[i,j] = sum(matb[i,:] * mat2b[:,j]);
				}
			}
			if(all(declassify(mat3) == control)){
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
	}
}

template<type T>
void diag_mat_multiplication(T data){
	{
		pd_a3p T[[2]] mat (0,0);
		pd_a3p T[[2]] mat2 (0,0);
		pd_a3p T[[2]] result = diagMatrixMultiplication(mat,mat2);
		pd_a3p T[[2]] mat3 (3,0);
		pd_a3p T[[2]] mat4 (0,0);
		result = diagMatrixMultiplication(mat3,mat4);
	}
	for(uint k = 3; k < 6;++k){
		for(uint l = 5; l > 2;--l){
			pd_a3p T[[2]] mat (k,l);
			pd_a3p T[[2]] mat2 (l,l);
			mat = randomize(mat); mat2 = randomize(mat2);
			for(uint i = 0; i < shape(declassify(mat2))[0]; ++i){
		        for(uint j = 0; j < shape(declassify(mat2))[1]; ++j){
		            if(i != j){
		                mat2[i,j] = 0;
		            }
		        }
		    }
		    T[[2]] matb = declassify(mat);
			T[[2]] mat2b = declassify(mat2);
		    pd_a3p T[[2]] mat3 (k,l) = diagMatrixMultiplication(mat,mat2);
		    T[[2]] control (k,l);
		    for(uint i = 0; i < k;++i){
		    	for(uint j = 0; j < l;++j){
		    		control[i,j] = matb[i,j] * mat2b[j,j];
		    	}

		    }
		    if(all(control == declassify(mat3))){
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
	}
}

template<type T>
void mat3D_multiplication(T data){
	for(uint k = 3; k < 6;++k){
		for(uint l = 5; l > 2;--l){
			pd_a3p T[[3]] mat (3,k,l);
			pd_a3p T[[3]] mat2 (3,l,k);
			mat = randomize(mat); mat2 = randomize(mat2);
			pd_a3p T[[3]] mat3 (3,k,k) = matrixMultiplication(mat,mat2);
			T[[3]] control (3,k,k);
			T[[3]] matb = declassify(mat);
			T[[3]] mat2b = declassify(mat2);
			for(uint h = 0; h < shape(mat3)[0];++h){
				for(uint i = 0; i < shape(declassify(mat3))[1];++i){
					for(uint j = 0; j < shape(declassify(mat3))[2];++j){
						control[h,i,j] = sum(matb[h,i,:] * mat2b[h,:,j]);
					}
				}
			}
			if(all(declassify(mat3) == control)){
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
	}
}

template<type T>
void diag3D_mat_multiplication(T data){
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
	for(uint k = 3; k < 6;++k){
		for(uint l = 5; l > 2;--l){
			pd_a3p T[[3]] mat (3,k,l);
			pd_a3p T[[3]] mat2 (3,l,l);
			mat = randomize(mat); mat2 = randomize(mat2);
			for(uint h = 0; h < shape(declassify(mat2))[0];++h){
				for(uint i = 0; i < shape(declassify(mat2))[1]; ++i){
		    	    for(uint j = 0; j < shape(declassify(mat2))[2]; ++j){
		        	    if(i != j){
		            	    mat2[h,i,j] = 0;
		            	}
		        	}
		    	}
		    }
		    pd_a3p T[[3]] mat3 (3,k,l) = diagMatrixMultiplication(mat,mat2);
		    T[[3]] control (3,k,l);
		    T[[3]] matb = declassify(mat);
			T[[3]] mat2b = declassify(mat2);
		    for(uint h = 0; h < shape(declassify(mat))[0];++h){
		    	for(uint i = 0; i < shape(declassify(mat))[1];++i){
		    		for(uint j = 0; j < shape(declassify(mat))[2];++j){
		    			control[h,i,j] = matb[h,i,j] * mat2b[h,j,j];
		    		}
		    	}
		    }
		    if(all(control == declassify(mat3))){
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
	}
}

void main(){
	print("A3P_Matrix test: start");

	print("TEST 1: Matrix row sums");
	{
		print("uint8");
		row_sum_test(0::uint8);
	}
	{
		print("uint16");
		row_sum_test(0::uint16);
	}
	{
		print("uint32");
		row_sum_test(0::uint32);
	}
	{
		print("uint64/uint");
		row_sum_test(0::uint);
	}
	{
		print("int8");
		row_sum_test(0::int8);
	}
	{
		print("int16");
		row_sum_test(0::int16);
	}
	{
		print("int32");
		row_sum_test(0::int32);
	}
	{
		print("int64/int");
		row_sum_test(0::int);
	}
	print("TEST 2: Matrix dotproduct for vectors");
	{
		print("uint8");
		dot_product(0::uint8);
	}
	{
		print("uint16");
		dot_product(0::uint16);
	}
	{
		print("uint32");
		dot_product(0::uint32);
	}
	{
		print("uint64/uint");
		dot_product(0::uint);
	}
	{
		print("int8");
		dot_product(0::int8);
	}
	{
		print("int16");
		dot_product(0::int16);
	}
	{
		print("int32");
		dot_product(0::int32);
	}
	{
		print("int64/int");
		dot_product(0::int);
	}
	print("TEST 3: Matrix dotproduct for matrices");
	{
		print("uint8");
		dot_product_matrix(0::uint8);
	}
	{
		print("uint16");
		dot_product_matrix(0::uint16);
	}
	{
		print("uint32");
		dot_product_matrix(0::uint32);
	}
	{
		print("uint64/uint");
		dot_product_matrix(0::uint);
	}
	{
		print("int8");
		dot_product_matrix(0::int8);
	}
	{
		print("int16");
		dot_product_matrix(0::int16);
	}
	{
		print("int32");
		dot_product_matrix(0::int32);
	}
	{
		print("int64/int");
		dot_product_matrix(0::int);
	}
	print("TEST 4: cross product for vectors");
	{
		print("int8");
		cross_product(0::int8);
	}
	{
		print("int16");
		cross_product(0::int16);
	}
	{
		print("int32");
		cross_product(0::int32);
	}
	{
		print("int64/int");
		cross_product(0::int);
	}
	print("TEST 5: cross product for matrices");
	{
		print("int8");
		cross_product_matrix(0::int8);
	}
	{
		print("int16");
		cross_product_matrix(0::int16);
	}
	{
		print("int32");
		cross_product_matrix(0::int32);
	}
	{
		print("int64/int");
		cross_product_matrix(0::int);
	}
	print("TEST 6: 2D Matrix multiplication");
	{
		print("uint8");
		mat_multiplication(0::uint8);
	}
	{
		print("uint16");
		mat_multiplication(0::uint16);
	}
	{
		print("uint32");
		mat_multiplication(0::uint32);
	}
	{
		print("uint64/uint");
		mat_multiplication(0::uint);
	}
	{
		print("int8");
		mat_multiplication(0::int8);
	}
	{
		print("int16");
		mat_multiplication(0::int16);
	}
	{
		print("int32");
		mat_multiplication(0::int32);
	}
	{
		print("int64/int");
		mat_multiplication(0::int);
	}
	print("TEST 7: 2D Diagonal matrix multiplication");
	{
		print("uint8");
		diag_mat_multiplication(0::uint8);
	}
	{
		print("uint16");
		diag_mat_multiplication(0::uint16);
	}
	{
		print("uint32");
		diag_mat_multiplication(0::uint32);
	}
	{
		print("uint64/uint");
		diag_mat_multiplication(0::uint);
	}
	{
		print("int8");
		diag_mat_multiplication(0::int8);
	}
	{
		print("int16");
		diag_mat_multiplication(0::int16);
	}
	{
		print("int32");
		diag_mat_multiplication(0::int32);
	}
	{
		print("int64/int");
		diag_mat_multiplication(0::int);
	}
	print("TEST 8: 3D Matrix multiplication");
	{
		print("uint8");
		mat3D_multiplication(0::uint8);
	}
	{
		print("uint16");
		mat3D_multiplication(0::uint16);
	}
	{
		print("uint32");
		mat3D_multiplication(0::uint32);
	}
	{
		print("uint64/uint");
		mat3D_multiplication(0::uint);
	}
	{
		print("int8");
		mat3D_multiplication(0::int8);
	}
	{
		print("int16");
		mat3D_multiplication(0::int16);
	}
	{
		print("int32");
		mat3D_multiplication(0::int32);
	}
	{
		print("int64/int");
		mat3D_multiplication(0::int);
	}
	print("TEST 9: 3D Diagonal matrix multiplication");
	{
		print("uint8");
		diag3D_mat_multiplication(0::uint8);
	}
	{
		print("uint16");
		diag3D_mat_multiplication(0::uint16);
	}
	{
		print("uint32");
		diag3D_mat_multiplication(0::uint32);
	}
	{
		print("uint64/uint");
		diag3D_mat_multiplication(0::uint);
	}
	{
		print("int8");
		diag3D_mat_multiplication(0::int8);
	}
	{
		print("int16");
		diag3D_mat_multiplication(0::int16);
	}
	{
		print("int32");
		diag3D_mat_multiplication(0::int32);
	}
	{
		print("int64/int");
		diag3D_mat_multiplication(0::int);
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}