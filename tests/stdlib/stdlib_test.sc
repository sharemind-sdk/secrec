/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module stdlib_test;

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
void test_flattening(T data){
	{
		T[[2]] mat (0,0);
		T[[1]] vec = flatten(mat);
	}
	bool result = true;
	pd_a3p T[[2]] temp (6,6);
	temp = randomize(temp);
	T[[2]] object = declassify(temp);
	T[[1]] vec = flatten(object);
	for(uint i = 0; i < shape(object)[0];++i){
		for(uint j = 0; j < shape(object)[1];++j){
			if(object[i,j] != vec[(6*i + j)]){
				result = false;
				break;
			}
		}
		if(!result){
			break;	
		}
	}
	if(!result){
		all_tests = all_tests +1;
		print("FAILURE! flattening returned: ");
		printVector(vec);
		print("From: ");
		printMatrix(object);
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<type T>
void equal_shapes_test(T data){
	{
		pd_a3p T[[2]] mat (0,0);
		pd_a3p T[[2]] mat2 (0,0);
		bool result = shapesAreEqual(mat,mat2);
		pd_a3p T[[2]] mat3 (0,2);
		pd_a3p T[[2]] mat4 (0,2);
		result = shapesAreEqual(mat3,mat4);
		pd_a3p T[[2]] mat5 (2,0);
		pd_a3p T[[2]] mat6 (2,0);
		result = shapesAreEqual(mat5,mat6);
	}
	{
		pd_a3p T[[2]] mat (5,5); 
		pd_a3p T[[2]] mat2 (5,5); 
		T[[2]] mat3 = declassify(randomize(mat));
		T[[2]] mat4 = declassify(randomize(mat2));
		bool result = shapesAreEqual(mat3,mat4);
		if(all(shape(mat3) == shape(mat4))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			all_tests = all_tests +1;
			print("FAILURE! Shapes should be equal but they're not");
		}
	}
	{
		pd_a3p T[[2]] mat (4,6);
		pd_a3p T[[2]] mat2 (24,3); 
		T[[2]] mat3 = declassify(randomize(mat));
		T[[2]] mat4 = declassify(randomize(mat2));
		bool result = shapesAreEqual(mat3,mat4);
		if(!all(shape(mat3) == shape(mat4))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			all_tests = all_tests +1;
			print("FAILURE! Shapes should not be equal but they are");
		}
	}
}

void test_any(){
	{
		bool[[1]] vec (0);
		bool result = any(vec);
	}
	bool result = true;
	bool[[1]] vec (6) = {true,false,true,true,false,false};
	bool[[1]] vec2 (6) = {true,false,false,false,false,false};
	bool[[1]] vec3 (6) = true;
	bool[[1]] vec4 (6) = false;
	bool control = any(vec);
	if(control != true){result = false;}
	
	control = any(vec2);
	if(control != true){result = false;}
	
	control = any(vec3);
	if(control != true){result = false;}
	
	control = any(vec4);
	if(control != false){result = false;}

	if(result){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Any failed");
	}
}

void test_any2(){
	bool result = true;
	bool[[1]] vec (6) = {true,false,true,true,false,false};
	bool[[1]] vec2 (6) = {true,false,false,false,false,true};
	bool[[1]] vec3 (6) = true;
	bool[[1]] vec4 (6) = false;
	bool[[1]] control = any(vec,2::uint);
	if(all(control) != true){result = false;}
	
	control = any(vec2,2::uint);
	if(all(control) != true){result = false;}
	
	control = any(vec3,2::uint);
	if(all(control) != true){result = false;}
	
	control = any(vec4,2::uint);
	if(all(control) != false){result = false;}

	if(result){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Any failed");
	}
}



void test_all(){
	{
		bool[[1]] vec (0);
		bool result = all(vec);
	}
	bool result = true;
	bool[[1]] vec (6) = {true,false,true,true,false,false};
	bool[[1]] vec2 (6) = {true,true,true,false,true,true};
	bool[[1]] vec3 (6) = true;
	bool[[1]] vec4 (6) = false;
	bool control = all(vec);
	if(control == true){result = false;}
	
	control = all(vec2);
	if(control == true){result = false;}
	
	control = all(vec3);
	if(control != true){result = false;}
	
	control = all(vec4);
	if(control == true){result = false;}

	if(result){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! All failed");
	}
}

void test_all2(){
	bool result = true;
	bool[[1]] vec (6) = {true,false,true,true,false,false};
	bool[[1]] vec2 (6) = {false,true,true,false,true,true};
	bool[[1]] vec3 (6) = true;
	bool[[1]] vec4 (6) = false;
	bool[[1]] control = all(vec,2::uint);
	if(any(control) == true){result = false;}
	
	control = all(vec2,2::uint);
	if(any(control) == true){result = false;}
	
	control = all(vec3,2::uint);
	if(any(control) == false){result = false;}
	
	control = all(vec4,2::uint);
	if(any(control) == true){result = false;}

	if(result){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! All failed");
	}
}

template<type T>
void test_sum(T data){
	{
		T[[1]] vec (0);
		T result = sum(vec);
	}
	pd_a3p T[[1]] temp (10); 
	T[[1]] vec = declassify(randomize(temp));
	T result = sum(vec);
	T control = 0;
	for(uint i = 0; i < size(vec);++i){
		control += vec[i];
	}
	if(all(control == result)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Sum failed");
	}
}


template<type T>
void test_sum2(T data){
	pd_a3p T[[1]] temp (10); 
	T[[1]] vec = declassify(randomize(temp));
	T[[1]] result = sum(vec,2::uint);
	uint startingIndex = 0;
	uint endIndex = size(vec) / 2;
	T[[1]] control (2)= 0;
	for(uint i = 0;i < 2;++i){
		for(uint j = startingIndex;j < endIndex;++j){
			control[i] += vec[j];
		}
		startingIndex = 5;
		endIndex = 10;
	}
	if(all(control == result)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Sum failed");
	}
}

template<type T>
void test_product(T data){
	{
		T[[1]] vec (0);
		T result = product(vec);
	}
	pd_a3p T[[1]] temp (10);
	T[[1]] vec = declassify(randomize(temp));
	T result = product(vec);
	T control = 1;
	for(uint i = 0; i < size(vec);++i){
		control *= vec[i];
	}
	if(control == result){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Sum failed");
	}
}

template<type T>
void test_product2(T data){
	pd_a3p T[[1]] temp (10);
	T[[1]] vec = declassify(randomize(temp));
	T[[1]] result = product(vec,2::uint);
	T[[1]] control (2)= 1;
	uint startingIndex = 0;
	uint endIndex = size(vec) / 2;
	for(uint i = 0; i < 2;++i){
		for(uint j = startingIndex; j < endIndex; ++j){
			control[i] *= vec[j];
		}
		startingIndex += size(vec) / 2;
		endIndex += size(vec) / 2;
	}
	if(all(control == result)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Product failed");
	}
}

template<type T>
void test_min(T data){
	pd_a3p T[[1]] temp (25);
	T[[1]] vec = declassify(randomize(temp));
	T result = min(vec);
	T control = 0;
	for(uint i = 0; i < size(vec);++i){
		if(i == 0){
			control = vec[i];
		}
		else{
			if(vec[i] < control){
				control = vec[i];
			}
		}
	} 
	if(control == result){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Min failed");
	}
}

template<type T>
void test_min2(T data){
	{
		T[[1]] vec (0);
		T[[1]] vec2 (0);
		T[[1]] result = min(vec,vec2);
	}
	pd_a3p T[[1]] temp (25);
	T[[1]] vec = declassify(randomize(temp));
	T[[1]] vec2 = declassify(randomize(temp));
	T[[1]] result = min(vec,vec2);
	T[[1]] control (25);
	for(uint i = 0; i < size(vec);++i){
		if(vec[i] < vec2[i]){
			control[i] = vec[i];
		}
		else{
			control[i] = vec2[i];
		}
	}
	if(all(control == result)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Min failed");
	}
}

template<type T>
void test_min3(T data){
	pd_a3p T[[1]] temp (20);
	T[[1]] vec = declassify(randomize(temp));
	T[[1]] result = min(vec,2::uint);
	T[[1]] control (2);
	uint startingIndex = 0;
	uint endIndex = 10;
	for(uint j = 0; j < 2;++j){
		for(uint i = startingIndex; i < endIndex;++i){
			if(i == startingIndex){
				control[j] = vec[i];
			}
			else{
				if(vec[i] < control[j]){
					control[j] = vec[i];
				}
			}
		}
		startingIndex += 10;
		endIndex += 10;
	}
	if(all(control == result)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Min failed");
	}
}

template<type T>
void test_max(T data){
	pd_a3p T[[1]] temp (25);
	T[[1]] vec = declassify(randomize(temp));
	T result = max(vec);
	T control = 0;
	for(uint i = 0; i < size(vec);++i){
		if(vec[i] > control){
			control = vec[i];
		}
	} 
	if(control == result){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Max failed");
	}
}

template<type T>
void test_max2(T data){
	{
		T[[1]] vec (0);
		T[[1]] vec2 (0);
		T[[1]] result = max(vec,vec2);
	}
	pd_a3p T[[1]] temp (25);
	T[[1]] vec = declassify(randomize(temp));
	T[[1]] vec2 = declassify(randomize(temp));
	T[[1]] result = max(vec,vec2);
	T[[1]] control (25);
	for(uint i = 0; i < size(vec);++i){
		if(vec[i] > vec2[i]){
			control[i] = vec[i];
		}
		else{
			control[i] = vec2[i];
		}
	}
	if(all(control == result)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Max failed");
	}
}

template<type T>
void test_max3(T data){
	pd_a3p T[[1]] temp (20);
	T[[1]] vec = declassify(randomize(temp));
	T[[1]] result = max(vec,2::uint);
	T[[1]] control (2);
	uint startingIndex = 0;
	uint endIndex = 10;
	for(uint j = 0; j < 2;++j){
		for(uint i = startingIndex; i < endIndex;++i){
			if(i == startingIndex){
				control[j] = vec[i];
			}
			else{
				if(vec[i] > control[j]){
					control[j] = vec[i];
				}
			}
		}
		startingIndex += 10;
		endIndex += 10;
	}
	if(all(control == result)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Max failed");
	}
}

void main(){
	print("Standard library test: start");

	print("TEST 1: Flattening utility");
	{
		print("bool");
		test_flattening(false);
	}
	{
		print("uint8");
		test_flattening(0::uint8);
	}
	{
		print("uint16");
		test_flattening(0::uint16);
	}
	{
		print("uint32");
		test_flattening(0::uint32);
	}
	{
		print("uint64/uint");
		test_flattening(0::uint);
	}
	{
		print("int8");
		test_flattening(0::int8);
	}
	{
		print("int16");
		test_flattening(0::int16);
	}
	{
		print("int32");
		test_flattening(0::int32);
	}
	{
		print("int64/int");
		test_flattening(0::int);
	}
	print("TEST 2: Shapes are equal utility");
	{
		print("bool");
		equal_shapes_test(false);
	}
	{
		print("uint8");
		equal_shapes_test(0::uint8);
	}
	{
		print("uint16");
		equal_shapes_test(0::uint16);
	}
	{
		print("uint32");
		equal_shapes_test(0::uint32);
	}
	{
		print("uint64/uint");
		equal_shapes_test(0::uint);
	}
	{
		print("int8");
		equal_shapes_test(0::int8);
	}
	{
		print("int16");
		equal_shapes_test(0::int16);
	}
	{
		print("int32");
		equal_shapes_test(0::int32);
	}
	{
		print("int64/int");
		equal_shapes_test(0::int);
	}
	print("TEST 3: All and Any functions");
	{
		print("Any");
		test_any();
	}
	{
		print("All");
		test_all();
	}
	print("TEST 4: All(parts) and Any(parts) functions");
	{
		print("Any");
		test_any2();
	}
	{
		print("All");
		test_all2();
	}
	print("TEST 5: Sum");
	{
		print("bool");
		pd_a3p bool[[1]] temp (10); 
		bool[[1]] vec = declassify(randomize(temp));
		uint result = sum(vec);
		uint control = 0;
		for(uint i = 0;i < size(vec);++i){
			if(vec[i] == true){
				control += 1;
			}
		}
		if(control == result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			all_tests = all_tests +1;
			print("FAILURE! Sum failed");
		}

	}
	{
		print("uint8");
		test_sum(0::uint8);
	}
	{
		print("uint16");
		test_sum(0::uint16);
	}
	{
		print("uint32");
		test_sum(0::uint32);
	}
	{
		print("uint64/uint");
		test_sum(0::uint);
	}
	{
		print("int8");
		test_sum(0::int8);
	}
	{
		print("int16");
		test_sum(0::int16);
	}
	{
		print("int32");
		test_sum(0::int32);
	}
	{
		print("int64/int");
		test_sum(0::int);
	}
	print("TEST 6: Sum (2)");
	{
		print("bool");
		pd_a3p bool[[1]] temp (10); 
		bool[[1]] vec = declassify(randomize(temp));
		uint[[1]] result = sum(vec,2::uint);
		uint[[1]] control (2) = 0;
		uint startingIndex = 0;
		uint endIndex = size(vec) / 2;
		for(uint i = 0;i < 2;++i){
			for(uint j = startingIndex;j < endIndex;++j){
				if(vec[j] == true){
					control[i] += 1;
				}
			}
			startingIndex = 5;
			endIndex = 10;
		}
		if(all(control == result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			all_tests = all_tests +1;
			print("FAILURE! Sum failed");
		}

	}
	{
		print("uint8");
		test_sum2(0::uint8);
	}
	{
		print("uint16");
		test_sum2(0::uint16);
	}
	{
		print("uint32");
		test_sum2(0::uint32);
	}
	{
		print("uint64/uint");
		test_sum2(0::uint);
	}
	{
		print("int8");
		test_sum2(0::int8);
	}
	{
		print("int16");
		test_sum2(0::int16);
	}
	{
		print("int32");
		test_sum2(0::int32);
	}
	{
		print("int64/int");
		test_sum2(0::int);
	}
	print("TEST 7: Product");
	{
		print("uint8");
		test_product(0::uint8);
	}
	{
		print("uint16");
		test_product(0::uint16);
	}
	{
		print("uint32");
		test_product(0::uint32);
	}
	{
		print("uint64/uint");
		test_product(0::uint);
	}
	{
		print("int8");
		test_product(0::int8);
	}
	{
		print("int16");
		test_product(0::int16);
	}
	{
		print("int32");
		test_product(0::int32);
	}
	{
		print("int64/int");
		test_product(0::int);
	}
	print("TEST 8: Product (2)");
	{
		print("uint8");
		test_product2(0::uint8);
	}
	{
		print("uint16");
		test_product2(0::uint16);
	}
	{
		print("uint32");
		test_product2(0::uint32);
	}
	{
		print("uint64/uint");
		test_product2(0::uint);
	}
	{
		print("int8");
		test_product2(0::int8);
	}
	{
		print("int16");
		test_product2(0::int16);
	}
	{
		print("int32");
		test_product2(0::int32);
	}
	{
		print("int64/int");
		test_product2(0::int);
	}
	print("TEST 9: Max");
	{
		print("uint8");
		test_max(0::uint8);
	}
	{
		print("uint16");
		test_max(0::uint16);
	}
	{
		print("uint32");
		test_max(0::uint32);
	}
	{
		print("uint64/uint");
		test_max(0::uint);
	}
	{
		print("int8");
		test_max(0::int8);
	}
	{
		print("int16");
		test_max(0::int16);
	}
	{
		print("int32");
		test_max(0::int32);
	}
	{
		print("int64/int");
		test_max(0::int);
	}
	print("TEST 10: Max pointwise");
	{
		print("uint8");
		test_max2(0::uint8);
	}
	{
		print("uint16");
		test_max2(0::uint16);
	}
	{
		print("uint32");
		test_max2(0::uint32);
	}
	{
		print("uint64/uint");
		test_max2(0::uint);
	}
	{
		print("int8");
		test_max2(0::int8);
	}
	{
		print("int16");
		test_max2(0::int16);
	}
	{
		print("int32");
		test_max2(0::int32);
	}
	{
		print("int64/int");
		test_max2(0::int);
	}
	print("TEST 11: Max (parts)");
	{
		print("uint8");
		test_max3(0::uint8);
	}
	{
		print("uint16");
		test_max3(0::uint16);
	}
	{
		print("uint32");
		test_max3(0::uint32);
	}
	{
		print("uint64/uint");
		test_max3(0::uint);
	}
	{
		print("int8");
		test_max3(0::int8);
	}
	{
		print("int16");
		test_max3(0::int16);
	}
	{
		print("int32");
		test_max3(0::int32);
	}
	{
		print("int64/int");
		test_max3(0::int);
	}
	print("TEST 12: Min");
	{
		print("uint8");
		test_min(0::uint8);
	}
	{
		print("uint16");
		test_min(0::uint16);
	}
	{
		print("uint32");
		test_min(0::uint32);
	}
	{
		print("uint64/uint");
		test_min(0::uint);
	}
	{
		print("int8");
		test_min(0::int8);
	}
	{
		print("int16");
		test_min(0::int16);
	}
	{
		print("int32");
		test_min(0::int32);
	}
	{
		print("int64/int");
		test_min(0::int);
	}
	print("TEST 13: Min pointwise");
	{
		print("uint8");
		test_min2(0::uint8);
	}
	{
		print("uint16");
		test_min2(0::uint16);
	}
	{
		print("uint32");
		test_min2(0::uint32);
	}
	{
		print("uint64/uint");
		test_min2(0::uint);
	}
	{
		print("int8");
		test_min2(0::int8);
	}
	{
		print("int16");
		test_min2(0::int16);
	}
	{
		print("int32");
		test_min2(0::int32);
	}
	{
		print("int64/int");
		test_min2(0::int);
	}
	print("TEST 14: Min (parts)");
	{
		print("uint8");
		test_min3(0::uint8);
	}
	{
		print("uint16");
		test_min3(0::uint16);
	}
	{
		print("uint32");
		test_min3(0::uint32);
	}
	{
		print("uint64/uint");
		test_min3(0::uint);
	}
	{
		print("int8");
		test_min3(0::int8);
	}
	{
		print("int16");
		test_min3(0::int16);
	}
	{
		print("int32");
		test_min3(0::int32);
	}
	{
		print("int64/int");
		test_min3(0::int);
	}


	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}