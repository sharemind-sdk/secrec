/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module shuffling_test;

import stdlib;
import matrix;
import additive3pp;
import a3p_matrix;
import a3p_oblivious;
import oblivious;
import a3p_random;
import a3p_sort;
import a3p_bloom;
import x3p_string;
import x3p_aes;
import x3p_join;
import profiling;
import xor3pp;
import test_utility;

domain pd_a3p additive3pp;

public uint all_tests;
public uint succeeded_tests;

template<type T>
void choice_test1(T data){
	{
		pd_a3p bool cond = true;
		pd_a3p T[[2]] mat (0,0);
		pd_a3p T[[2]] mat2 (0,0);
		pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
		pd_a3p T[[2]] mat4 (0,2);
		pd_a3p T[[2]] mat5 (0,2);
		mat3 = choose(cond,mat4,mat5);
		pd_a3p T[[2]] mat6 (2,0);
		pd_a3p T[[2]] mat7 (2,0);
		mat3 = choose(cond,mat6,mat7);
	}
	pd_a3p bool cond = true;
	pd_a3p T[[2]] mat (10,10);
	pd_a3p T[[2]] mat2 (10,10);
	mat = randomize(mat); mat2 = randomize(mat2);
	pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
	if(all(declassify(mat) == declassify(mat3))){
		succeeded_tests = succeeded_tests + 1;
		all_tests = all_tests +1;
		print("SUCCESS!");
	}
	else{
		print("FAILURE! choose test failed. Expected: ", arrayToString(flatten(declassify(mat))), " Got: ",arrayToString(flatten(declassify(mat3))));
 		all_tests = all_tests +1;
	}
	cond = false;
	mat = randomize(mat); mat2 = randomize(mat2);
	mat3 = choose(cond,mat,mat2);
	if(all(declassify(mat2) == declassify(mat3))){
		succeeded_tests = succeeded_tests + 1;
		all_tests = all_tests +1;
		print("SUCCESS!");
	}
	else{
		print("FAILURE! choose test failed. Expected: ", arrayToString(flatten(declassify(mat2))), " Got: ",arrayToString(flatten(declassify(mat3))));
 		all_tests = all_tests +1;
	}
}

template<domain D:additive3pp,type T>
void choice_test1_xor(D T data){
	pd_a3p bool[[2]] cond (10,10) = true;
	pd_a3p T[[2]] mat (10,10);
	pd_a3p T[[2]] mat2 (10,10);
	mat = randomize(mat); mat2 = randomize(mat2);
	pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
	if(all(declassify(mat) == declassify(mat3))){
		succeeded_tests = succeeded_tests + 1;
		all_tests = all_tests +1;
		print("SUCCESS!");
	}
	else{
		print("FAILURE! choose test failed. Expected: ", arrayToString(flatten(declassify(mat))), " Got: ",arrayToString(flatten(declassify(mat3))));
 		all_tests = all_tests +1;
	}
	cond = false;
	mat = randomize(mat); mat2 = randomize(mat2);
	mat3 = choose(cond,mat,mat2);
	if(all(declassify(mat2) == declassify(mat3))){
		succeeded_tests = succeeded_tests + 1;
		all_tests = all_tests +1;
		print("SUCCESS!");
	}
	else{
		print("FAILURE! choose test failed. Expected: ", arrayToString(flatten(declassify(mat2))), " Got: ",arrayToString(flatten(declassify(mat3))));
 		all_tests = all_tests +1;
	}
}

template<type T>
void choice_test2(T data){
	{
		pd_a3p bool[[2]] cond (0,0);
		pd_a3p T[[2]] mat (0,0);
		pd_a3p T[[2]] mat2 (0,0);
		pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
		pd_a3p bool[[2]] cond2 (0,2);
		pd_a3p T[[2]] mat4 (0,2);
		pd_a3p T[[2]] mat5 (0,2);
		mat3 = choose(cond2,mat4,mat5);
		pd_a3p bool[[2]] cond3 (2,0);
		pd_a3p T[[2]] mat6 (2,0);
		pd_a3p T[[2]] mat7 (2,0);
		mat3 = choose(cond3,mat6,mat7);
	}
	bool result = true;
	uint column;
	pd_a3p bool[[2]] cond (6,6);
	pd_a3p T[[2]] mat (6,6);
	pd_a3p T[[2]] mat2 (6,6);
	mat = randomize(mat); mat2 = randomize(mat2); cond = randomize(cond);
	pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
	for(uint i = 0; i < 6;++i){
		for(uint j = 0; j < 6; ++j){
			if(declassify(cond[i,j])){
				if(!(declassify(mat3[i,j]) == declassify(mat[i,j]))){
					result = false;
					column = j;
				}
			}
			else{
				if(!(declassify(mat3[i,j]) == declassify(mat2[i,j]))){
					result = false;
					column = j;
				}
			}
			if(!result){
				break;
			}
		}
		if(!result){
			print("FAILURE! choose test failed. Expected: ", declassify(mat[i,column]), " Got: ", declassify(mat3[i,column]));
 			all_tests = all_tests +1;
			break;
		}
	}
	if(result){
		succeeded_tests = succeeded_tests + 1;
		all_tests = all_tests +1;
		print("SUCCESS!");
	}
}

template<domain D:additive3pp,type T>
void choice_test2_xor(D T data){
	bool result = true;
	uint column;
	pd_a3p bool[[2]] cond (6,6);
	pd_a3p T[[2]] mat (6,6);
	pd_a3p T[[2]] mat2 (6,6);
	mat = randomize(mat); mat2 = randomize(mat2); cond = randomize(cond);
	pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
	for(uint i = 0; i < 6;++i){
		for(uint j = 0; j < 6; ++j){
			if(declassify(cond[i,j])){
				if(!(declassify(mat3[i,j]) == declassify(mat[i,j]))){
					result = false;
					column = j;
				}
			}
			else{
				if(!(declassify(mat3[i,j]) == declassify(mat2[i,j]))){
					result = false;
					column = j;
				}
			}
			if(!result){
				break;
			}
		}
		if(!result){
			print("FAILURE! choose test failed. Expected: ", declassify(mat[i,column]), " Got: ", declassify(mat3[i,column]));
 			all_tests = all_tests +1;
			break;
		}
	}
	if(result){
		succeeded_tests = succeeded_tests + 1;
		all_tests = all_tests +1;
		print("SUCCESS!");
	}
}

template<type T>
void vector_lookup_test(T data){
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p uint index = 0;
		pd_a3p T scalar = vectorLookup(vec,index);
	}
	bool result = true;
	pd_a3p T scalar;
	pd_a3p T control;
	pd_a3p uint index;
	for(uint i = 3; i < 9;++i){
		pd_a3p T[[1]] vec (i);
		vec = randomize(vec);
		for(uint j = 0; j < size(vec); ++j){
			index = j;
			scalar = vectorLookup(vec,index);
			if(declassify(scalar) != declassify(vec[j])){
				control = vec[j];
				result = false;
				break;
			}
		}
		if(!result){
			print("FAILURE! vectorLookup failed. Expected: ", declassify(control), " Got: ",declassify(scalar));
 			all_tests = all_tests +1;
 			result = true;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
 			all_tests = all_tests +1;
 			print("SUCCESS!");
		}
	}
}

template<type T>
void matrix_row_lookup(T data){
	/**
	\todo matrixLookupRow fails when given an empty vector
	{
		pd_a3p T[[2]] mat (0,0);
		pd_a3p uint index = 0;
		pd_a3p T[[1]] row = matrixLookupRow(mat,index);
		pd_a3p T[[2]] mat2 (2,0);
		row = matrixLookupRow(mat2,index);
		pd_a3p T[[2]] mat3 (0,2);
		row = matrixLookupRow(mat3,index);
	}*/
	bool result = true;
	pd_a3p T[[2]] mat (5,5);
	mat = randomize(mat);
	pd_a3p uint index;
	pd_a3p T[[1]] row;
	public T[[1]] control;
	for(uint i = 0; i < 5; ++i){
		index = i;
		row = matrixLookupRow(mat,index);
		control = declassify(mat[i,:]);
		if(!all(declassify(row) == control)){
			result = false;
			break;
		}
	}
	if(!result){
		print("FAILURE! Matrix row lookup failed. Expected: ", arrayToString(control), " Got: ",arrayToString(declassify(row)));
 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<type T>
void matrix_col_lookup(T data){
	/**
	\todo matrixLookupColumn fails when given an empty vector
	{
		pd_a3p T[[2]] mat (0,0);
		pd_a3p uint index = 0;
		pd_a3p T[[1]] row = matrixLookupColumn(mat,index);
		pd_a3p T[[2]] mat2 (2,0);
		row = matrixLookupColumn(mat2,index);
		pd_a3p T[[2]] mat3 (0,2);
		row = matrixLookupColumn(mat3,index);
	}*/
	bool result = true;
	pd_a3p T[[2]] mat (5,5);
	mat = randomize(mat);
	pd_a3p uint index;
	pd_a3p T[[1]] col;
	public T[[1]] control;
	for(uint i = 0; i < 5; ++i){
		index = i;
		col = matrixLookupColumn(mat,index);
		control = declassify(mat[:,i]);
		if(!all(declassify(col) == control)){
			result = false;
			break;
		}
	}
	if(!result){
		print("FAILURE! Matrix column lookup failed. Expected: ", arrayToString(control), " Got: ",arrayToString(declassify(col)));
 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<type T>
void matrix_lookup(T data){
	{
		pd_a3p uint index1 = 0;
		pd_a3p uint index2 = 0;
		pd_a3p T[[2]] mat2 (2,0);
		pd_a3p T result = matrixLookup(mat2,index1,index2);
	}
	bool result = true;
	pd_a3p T[[2]] mat (5,5);
	mat = randomize(mat);
	pd_a3p uint row_index;
	pd_a3p uint col_index;
	pd_a3p T nr;
	public T control;
	for(uint i = 0; i < 5; ++i){
		row_index = i;
		for(uint j = 0; j < 5; ++j){
			col_index = j;
			nr = matrixLookup(mat,row_index,col_index);
			control = declassify(mat[i,j]);
			if(!(declassify(nr) == control)){
				result = false;
				break;
			}
		}
		if(!result){
			break;
		}	
	}
	if(!result){
		print("FAILURE! Matrix lookup failed. Expected: ", control, " Got: ", declassify(nr));
 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template <type T>
void vector_update(T data){
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p uint index = 0;
		pd_a3p T newValue;
		vec = vectorUpdate(vec,index,newValue);
	}
	bool result = true;
	pd_a3p T[[1]] rand (1);
	rand = randomize(rand);
	pd_a3p T scalar = rand[0];
	pd_a3p uint index;
	pd_a3p T control;
	for(uint i = 3; i < 9; ++i){
		pd_a3p T[[1]] vec (i);
		vec = randomize(vec);
		for(uint j = 0; j < size(vec);++j){
			index = j;
			vec = vectorUpdate(vec,index,scalar);
			if(declassify(vec[j]) != declassify(scalar)){
				control = vec[j];
				result = false;
				break;
			}
		}
		if(!result){
			print("FAILURE! vector update failed on size ",i," Expected: ", declassify(scalar), " Got: ", declassify(control));
	 		all_tests = all_tests +1;
	 		result = true;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
 			all_tests = all_tests +1;
 			print("SUCCESS!");
		}
	} 
}

template<type T>
void matrix_row_update(T data){
	bool result = true;
	pd_a3p T[[2]] mat (5,5);
	mat = randomize(mat);

	pd_a3p T[[1]] vec (5);
	vec = randomize(vec);

	pd_a3p uint row_index;

	for(uint i = 0; i < 5; ++i){
		row_index = i;
		mat = matrixUpdateRow(mat,row_index,vec);
		if(!all(declassify(vec) == declassify(mat[i,:]))){
			result = false;
		}
		if(!result){
			print("FAILURE! Matrix row update failed. Expected: ", arrayToString(declassify(vec)), " Got: ",arrayToString(flatten(declassify(mat[declassify(row_index),:]))));
	 		all_tests = all_tests +1;
	 		break;
 		}
 		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
}

template<type T>
void matrix_col_update(T data){
	bool result = true;
	pd_a3p T[[2]] mat (5,5);
	mat = randomize(mat);

	pd_a3p T[[1]] vec (5);
	vec = randomize(vec);

	pd_a3p uint col_index;

	for(uint i = 0; i < 5; ++i){
		col_index = i;
		mat = matrixUpdateColumn(mat,col_index,vec);
		if(!all(declassify(vec) == declassify(mat[:,i]))){
			result = false;
		}
		if(!result){
			print("FAILURE! Matrix column update failed. Expected: ", arrayToString(declassify(vec)), " Got: ",arrayToString(flatten(declassify(mat[:,declassify(col_index)]))));
	 		all_tests = all_tests +1;
	 		break;
 		}
 		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
}

template<type T>
void matrix_update(T data){
	bool result = true;
	pd_a3p T[[2]] mat (5,5);
	mat = randomize(mat);

	pd_a3p T[[1]] vec (1);
	vec = randomize(vec);

	pd_a3p T scalar = vec[0];
	pd_a3p uint row_index;
	pd_a3p uint col_index;

	for(uint i = 0; i < 5; ++i){
		row_index = i;
		for(uint j = 0; j < 5; ++j){
			col_index = j;
			mat = matrixUpdate(mat,row_index,col_index,scalar);
			if(!(declassify(scalar) == declassify(mat[i,j]))){
				result = false;
			}
			if(!result){
				break;
			}
		}
		if(!result){
				break;
		}
	}
	if(!result){
		print("FAILURE! Matrix update failed. Expected: ", declassify(scalar), " Got: ",declassify(mat[declassify(row_index),declassify(col_index)]));
 		all_tests = all_tests +1;
	}
		else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

void main(){
	public int8 INT8_MAX = 127;
	public int8 INT8_MIN = -128;
	public int16 INT16_MAX = 32767;
	public int16 INT16_MIN = -32768;
	public int32 INT32_MAX = 2147483647;
	public int32 INT32_MIN = -2147483648;
	public int64 INT64_MAX = 9223372036854775807;
	public int64 INT64_MIN = -9223372036854775808;
 
 	public uint8 UINT8_MAX = 255; //2^8 - 1
 	public uint16 UINT16_MAX = 65535; // 2^16 - 1
 	public uint32 UINT32_MAX = 4294967295; // 2^32 - 1
 	public uint64 UINT64_MAX = 18446744073709551615; //2^64 - 1


	print("Oblivious test: start");

	print("TEST 1a: Oblivious choice scalar condition");
	{
		print("bool");
		choice_test1(false);
	}
	{
		print("uint8");
		choice_test1(0::uint8);
	}
	{
		print("uint16");
		choice_test1(0::uint16);
	}
	{
		print("uint32");
		choice_test1(0::uint32);
	}
	{
		print("uint64/uint");
		choice_test1(0::uint);
	}
	{
		print("int8");
		choice_test1(0::int8);
	}
	{
		print("int16");
		choice_test1(0::int16);
	}
	{
		print("int32");
		choice_test1(0::int32);
	}
	{
		print("int64/int");
		choice_test1(0::int);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		choice_test1_xor(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		choice_test1_xor(data);
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 data = 0;
		choice_test1_xor(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		choice_test1_xor(data);
	}
	print("TEST 1b: Oblivious choice matrix condition");
	{
		print("bool");
		choice_test2(false);
	}
	{
		print("uint8");
		choice_test2(0::uint8);
	}
	{
		print("uint16");
		choice_test2(0::uint16);
	}
	{
		print("uint32");
		choice_test2(0::uint32);
	}
	{
		print("uint64/uint");
		choice_test2(0::uint);
	}
	{
		print("int8");
		choice_test2(0::int8);
	}
	{
		print("int16");
		choice_test2(0::int16);
	}
	{
		print("int32");
		choice_test2(0::int32);
	}
	{
		print("int64/int");
		choice_test2(0::int);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		choice_test2_xor(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		choice_test2_xor(data);
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 data = 0;
		choice_test2_xor(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		choice_test2_xor(data);
	}
	print("TEST 2: Oblivious vector lookup with 3-8 element vectors");
	{
		print("bool");
		vector_lookup_test(false);
	}
	{
		print("uint8");
		vector_lookup_test(0::uint8);
	}
	{
		print("uint16");
		vector_lookup_test(0::uint16);
	}
	{
		print("uint32");
		vector_lookup_test(0::uint32);
	}
	{
		print("uint64/uint");
		vector_lookup_test(0::uint);
	}
	{
		print("int8");
		vector_lookup_test(0::int8);
	}
	{
		print("int16");
		vector_lookup_test(0::int16);
	}
	{
		print("int32");
		vector_lookup_test(0::int32);
	}
	{
		print("int64/int");
		vector_lookup_test(0::int);
	}
	print("TEST 3: Oblivious matrix row lookup in (5,5) matrix");
	{
		print("bool");
		matrix_row_lookup(false);
	}
	{
		print("uint8");
		matrix_row_lookup(0::uint8);
	}
	{
		print("uint16");
		matrix_row_lookup(0::uint16);
	}
	{
		print("uint32");
		matrix_row_lookup(0::uint32);
	}
	{
		print("uint64/uint");
		matrix_row_lookup(0::uint);
	}
	{
		print("int8");
		matrix_row_lookup(0::int8);
	}
	{
		print("int16");
		matrix_row_lookup(0::int16);
	}
	{
		print("int32");
		matrix_row_lookup(0::int32);
	}
	{
		print("int64/int");
		matrix_row_lookup(0::int);
	}
	print("TEST 4: Oblivious matrix column lookup in (5,5) matrix");
	{
		print("bool");
		matrix_col_lookup(false);
	}
	{
		print("uint8");
		matrix_col_lookup(0::uint8);
	}
	{
		print("uint16");
		matrix_col_lookup(0::uint16);
	}
	{
		print("uint32");
		matrix_col_lookup(0::uint32);
	}
	{
		print("uint64/uint");
		matrix_col_lookup(0::uint);
	}
	{
		print("int8");
		matrix_col_lookup(0::int8);
	}
	{
		print("int16");
		matrix_col_lookup(0::int16);
	}
	{
		print("int32");
		matrix_col_lookup(0::int32);
	}
	{
		print("int64/int");
		matrix_col_lookup(0::int);
	}
	print("TEST 5: Oblivious matrix lookup in (5,5) matrix");
	{
		print("bool");
		matrix_lookup(false);
	}
	{
		print("uint8");
		matrix_lookup(0::uint8);
	}
	{
		print("uint16");
		matrix_lookup(0::uint16);
	}
	{
		print("uint32");
		matrix_lookup(0::uint32);
	}
	{
		print("uint64/uint");
		matrix_lookup(0::uint);
	}
	{
		print("int8");
		matrix_lookup(0::int8);
	}
	{
		print("int16");
		matrix_lookup(0::int16);
	}
	{
		print("int32");
		matrix_lookup(0::int32);
	}
	{
		print("int64/int");
		matrix_lookup(0::int);
	}
	print("TEST 6: Oblivious vector update in 3-8 element vector");
	{
		print("bool");
		vector_update(false);
	}
	{
		print("uint8");
		vector_update(0::uint8);
	}
	{
		print("uint16");
		vector_update(0::uint16);
	}
	{
		print("uint32");
		vector_update(0::uint32);
	}
	{
		print("uint64/uint");
		vector_update(0::uint);
	}
	{
		print("int8");
		vector_update(0::int8);
	}
	{
		print("int16");
		vector_update(0::int16);
	}
	{
		print("int32");
		vector_update(0::int32);
	}
	{
		print("int64/int");
		vector_update(0::int);
	}
	print("TEST 7: Oblivious matrix row update");
	{
		print("bool");
		matrix_row_update(false);
	}
	{
		print("uint8");
		matrix_row_update(0::uint8);
	}
	{
		print("uint16");
		matrix_row_update(0::uint16);
	}
	{
		print("uint32");
		matrix_row_update(0::uint32);
	}
	{
		print("uint64/uint");
		matrix_row_update(0::uint);
	}
	{
		print("int8");
		matrix_row_update(0::int8);
	}
	{
		print("int16");
		matrix_row_update(0::int16);
	}
	{
		print("int32");
		matrix_row_update(0::int32);
	}
	{
		print("int64/int");
		matrix_row_update(0::int);
	}
	print("TEST 8: Oblivious matrix column update");
	{
		print("bool");
		matrix_col_update(false);
	}
	{
		print("uint8");
		matrix_col_update(0::uint8);
	}
	{
		print("uint16");
		matrix_col_update(0::uint16);
	}
	{
		print("uint32");
		matrix_col_update(0::uint32);
	}
	{
		print("uint64/uint");
		matrix_col_update(0::uint);
	}
	{
		print("int8");
		matrix_col_update(0::int8);
	}
	{
		print("int16");
		matrix_col_update(0::int16);
	}
	{
		print("int32");
		matrix_col_update(0::int32);
	}
	{
		print("int64/int");
		matrix_col_update(0::int);
	}
	print("TEST 9: Oblivious matrix update");
	{
		print("bool");
		matrix_update(false);
	}
	{
		print("uint8");
		matrix_update(0::uint8);
	}
	{
		print("uint16");
		matrix_update(0::uint16);
	}
	{
		print("uint32");
		matrix_update(0::uint32);
	}
	{
		print("uint64/uint");
		matrix_update(0::uint);
	}
	{
		print("int8");
		matrix_update(0::int8);
	}
	{
		print("int16");
		matrix_update(0::int16);
	}
	{
		print("int32");
		matrix_update(0::int32);
	}
	{
		print("int64/int");
		matrix_update(0::int);
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);

    test_report(all_tests, succeeded_tests);
}
