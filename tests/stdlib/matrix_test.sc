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

void test_unit_matrix(){
	bool result = true;
	for(uint i = 1; i < 10;++i){
		uint[[2]] mat = unitMatrix(i);
		for(uint j = 0; j < shape(mat)[0];++j){
			for(uint k = 0; k < shape(mat)[0];++k){
				if(j == k){
					if(mat[j,k] != 1){
						result = false;
					}
				}
				else{
					if(mat[j,k] != 0){
						result = false;
					}
				}
			}
		}
		if(result){
	 		succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
	 	}
	 	else{
	 		print("FAILURE! did not return unit matrix at size", i);
			all_tests = all_tests +1;
		}
	}	
}

template<domain D : additive3pp,type T>
void test_transpose(D T data){
	bool result = true;
	pd_a3p T[[2]] mat (6,6);
	mat = randomize(mat);
	pd_a3p T[[2]] mat2 = transpose(mat);
	for(uint i = 0; i < 6; ++i){
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
	bool result = true;
	pd_a3p T[[3]] mat (5,5,5);
	mat = randomize(mat);
	pd_a3p T[[3]] mat2 = transpose(mat);
	for(uint i = 0; i < 5; ++i){
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
	pd_a3p T[[2]] mat (5,5);
	T[[1]] control (5);
	mat = randomize(mat);
	T[[1]] row_sums = rowSums(declassify(mat));
	for(uint i = 0; i < 5;++i){
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
	pd_a3p T[[1]] vec (15);
	pd_a3p T[[1]] vec2 (15);
	vec = randomize(vec); vec2 = randomize(vec2);
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
	pd_a3p T[[2]] mat (10,10);
	pd_a3p T[[2]] mat2 (10,10);
	mat = randomize(mat); mat2 = randomize(mat2);
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
	vec = randomize(vec); vec2 = randomize(vec2);
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
	pd_a3p T[[2]] mat (3,3);
	pd_a3p T[[2]] mat2 (3,3);
	mat = randomize(mat); mat2 = randomize(mat2);
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
	for(uint k = 3; k < 6;++k){
		for(uint l = 5; l > 2;--l){
			pd_a3p T[[2]] mat (k,l);
			pd_a3p T[[2]] mat2 (l,k);
			mat = randomize(mat); mat2 = randomize(mat2);
			T[[2]] mat3 (k,k) = matrixMultiplication(declassify(mat),declassify(mat2));
			T[[2]] control (k,k);
			for(uint i = 0; i < shape(mat3)[0];++i){
				for(uint j = 0; j < shape(mat3)[1];++j){
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
	}
}


void main(){
	print("Matrix test: start");

	print("TEST 0: Unit matrix");
	{
		test_unit_matrix();
	}

	print("TEST 1: Matrix transpose 2D and 3D");
	{
		print("bool");
		pd_a3p bool data = false;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("uint8");
		pd_a3p uint8 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("uint16");
		pd_a3p uint16 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("uint32");
		pd_a3p uint32 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("uint64/uint");
		pd_a3p uint64 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("int8");
		pd_a3p int8 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("int16");
		pd_a3p int16 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("int32");
		pd_a3p int32 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("int64/int");
		pd_a3p int data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		test_transpose(data);
		test_transpose2(data);
	}
	print("TEST 2: Matrix row sums");
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
	// Testing matrix colSums is useless because colSums consists of 2 functions: transpose and rowSums
	// which have already been tested

	print("TEST 3: Dot product for vectors");
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
	print("TEST 4: Dot product for matrices");
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
	print("TEST 5: cross product for vectors");
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
	print("TEST 6: cross product for matrices");
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
	print("TEST 7: Matrix multiplication");
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

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}