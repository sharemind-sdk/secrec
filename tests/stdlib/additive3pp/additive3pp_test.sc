/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module sorting_test;

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
import xor3pp;
import test_utility;

domain pd_a3p additive3pp;

public uint all_tests;
public uint succeeded_tests;

template<type T>
void test_sign(T data){
	{
		pd_a3p T[[1]] vec (0);
		vec = sign(vec);
	}
	pd_a3p T[[1]] temp (15);
	temp = randomize(temp);
	T[[1]] vec = declassify(temp);
	T[[1]] result = declassify(sign(temp));
	T[[1]] control (size(result));
	for(uint i = 0; i < size(control);++i){
		if(vec[i] < 0){
			control[i] = -1;
		}
		if(vec[i] > 0){
			control[i] = 1;
		}
		if(vec[i] == 0){
			control[i] = 0;	
		}
	} 
	if(all(result == control)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Sign failed");
	}
}

template<type T,type T2>
void test_abs(T data, T2 data2){
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p T2[[1]] vec2 = abs(vec);
	}
	pd_a3p T[[1]] temp (15);
	temp = randomize(temp);
	T[[1]] vec = declassify(temp);
	T2[[1]] result = declassify(abs(temp));
	T2[[1]] control (size(result));
	for(uint i = 0; i < size(control);++i){
		if(vec[i] < 0){
			control[i] = (T2)(-vec[i]);
		}
		if(vec[i] >= 0){
			control[i] = (T2)(vec[i]);
		}
	} 
	if(all(result == control)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Sign failed");
	}
}

template<type T>
void test_sum(T data){
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p T vec2 = sum(vec);
	}
	pd_a3p T[[1]] temp (10);
	temp = randomize(temp); 
	T[[1]] vec = declassify(temp);
	T result = declassify(sum(temp));
	T control = 0;
	for(uint i = 0; i < size(vec);++i){
		control += vec[i];
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
void test_sum2(T data){
	pd_a3p T[[1]] temp (10); 
	temp = randomize(temp);
	T[[1]] vec = declassify(temp);
	T[[1]] result = declassify(sum(temp,2::uint));
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
	/**
	\todo product does not take 0 element vectors as parameters
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p T vec2 = product(vec); 
	}*/
	pd_a3p T[[1]] temp (10);
	temp = randomize(temp);
	T[[1]] vec = declassify(temp);
	T result = declassify(product(temp));
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
	temp = randomize(temp);
	T[[1]] vec = declassify(temp);
	T[[1]] result = declassify(product(temp,2::uint));
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

void test_any(){
	{
		pd_a3p bool[[1]] vec (0);
		pd_a3p bool result = any(vec);
	}
	bool result = true;
	pd_a3p bool[[1]] vec (6) = {true,false,true,true,false,false};
	pd_a3p bool[[1]] vec2 (6) = {true,false,false,false,false,false};
	pd_a3p bool[[1]] vec3 (6) = true;
	pd_a3p bool[[1]] vec4 (6) = false;
	pd_a3p bool control = any(vec);
	if(declassify(control) != true){result = false;}
	
	control = any(vec2);
	if(declassify(control) != true){result = false;}
	
	control = any(vec3);
	if(declassify(control) != true){result = false;}
	
	control = any(vec4);
	if(declassify(control) != false){result = false;}

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
		pd_a3p bool[[1]] vec (0);
		pd_a3p bool result = all(vec);
	}
	bool result = true;
	pd_a3p bool[[1]] vec (6) = {true,false,true,true,false,false};
	pd_a3p bool[[1]] vec2 (6) = {true,true,true,false,true,true};
	pd_a3p bool[[1]] vec3 (6) = true;
	pd_a3p bool[[1]] vec4 (6) = false;
	pd_a3p bool control = all(vec);
	if(declassify(control) == true){result = false;}
	
	control = all(vec2);
	if(declassify(control) == true){result = false;}
	
	control = all(vec3);
	if(declassify(control) != true){result = false;}
	
	control = all(vec4);
	if(declassify(control) == true){result = false;}

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
void test_min(T data){
	pd_a3p T[[1]] temp (25);
	temp = randomize(temp);
	T[[1]] vec = declassify(temp);
	T result = declassify(min(temp));
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
	pd_a3p T[[1]] temp (25);
	temp = randomize(temp);
	T[[1]] vec = declassify(temp);
	T[[1]] result = declassify(min(temp,5::uint));
	T[[1]] control (5)= 0;
	uint startingIndex = 0;
	uint endIndex = 5;
	for(uint i = 0; i < 5; ++i){
		for(uint j = startingIndex; j < endIndex;++j){
			if(j == startingIndex){
				control[i] = vec[j];
			}
			else{
				if(vec[j] < control[i]){
					control[i] = vec[j];
				}
			}
		}
		startingIndex += 5;
		endIndex += 5; 
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
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p T[[1]] vec2 (0);
		pd_a3p T[[1]] result = min(vec,vec2); 
		pd_a3p T[[2]] mat (0,0);
		pd_a3p T[[2]] mat2 (0,0);
		pd_a3p T[[2]] result2 = min(mat,mat2); 
		pd_a3p T[[2]] mat3 (0,2);
		pd_a3p T[[2]] mat4 (0,2);
		result2 = min(mat3,mat4);
		pd_a3p T[[2]] mat5 (2,0);
		pd_a3p T[[2]] mat6 (2,0);
		result2 = min(mat5,mat6);  
	}
	pd_a3p T[[1]] temp (10);
	pd_a3p T[[1]] temp2 (10);
	temp = randomize(temp);
	temp2 = randomize(temp2);
	T[[1]] vec = declassify(temp);
	T[[1]] vec2 = declassify(temp2);
	T[[1]] result = declassify(min(temp,temp2));
	T[[1]] control (10) = 0;
	for(uint i = 0; i < size(vec);++i){
		if(vec[i] <= vec2[i]){
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
void test_max(T data){
	pd_a3p T[[1]] temp (25);
	temp = randomize(temp);
	T[[1]] vec = declassify(temp);
	T result = declassify(max(temp));
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
	pd_a3p T[[1]] temp (25);
	temp = randomize(temp);
	T[[1]] vec = declassify(temp);
	T[[1]] result = declassify(max(temp,5::uint));
	T[[1]] control (5)= 0;
	uint startingIndex = 0;
	uint endIndex = 5;
	for(uint i = 0; i < 5; ++i){
		for(uint j = startingIndex; j < endIndex;++j){
			if(j == startingIndex){
				control[i] = vec[j];
			}
			else{
				if(vec[j] > control[i]){
					control[i] = vec[j];
				}
			}
		}
		startingIndex += 5;
		endIndex += 5; 
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
	{
		pd_a3p T[[1]] vec (0);
		pd_a3p T[[1]] vec2 (0);
		pd_a3p T[[1]] result = min(vec,vec2); 
		pd_a3p T[[2]] mat (0,0);
		pd_a3p T[[2]] mat2 (0,0);
		pd_a3p T[[2]] result2 = min(mat,mat2); 
		pd_a3p T[[2]] mat3 (0,2);
		pd_a3p T[[2]] mat4 (0,2);
		result2 = min(mat3,mat4);
		pd_a3p T[[2]] mat5 (2,0);
		pd_a3p T[[2]] mat6 (2,0);
		result2 = min(mat5,mat6);  
	}
	pd_a3p T[[1]] temp (10);
	pd_a3p T[[1]] temp2 (10);
	temp = randomize(temp);
	temp2 = randomize(temp2);
	T[[1]] vec = declassify(temp);
	T[[1]] vec2 = declassify(temp2);
	T[[1]] result = declassify(max(temp,temp2));
	T[[1]] control (10) = 0;
	for(uint i = 0; i < size(vec);++i){
		if(vec[i] >= vec2[i]){
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

void test_floor(){
	//scalar
	pd_a3p float64[[1]] value = {15.892356329,5.12974291,7.5009235790235,-52.325623,-12.5002362,-1,873258};
	int64[[1]] control = {15,5,7,-53,-13,-2};
	for(uint i = 0; i < size(value); ++i){
		int64 result = declassify(floor(value[i]));
		if(control[i] == result){
			print("SUCCESS");
			all_tests += 1;
			succeeded_tests += 1;
		}
		else{
			print("FAILURE! Floor failed");
			all_tests += 1;
		}
	}
	//vector
	int64[[1]] result = declassify(floor(value));
		if(all(control == result)){
		print("SUCCESS");
		all_tests += 1;
		succeeded_tests += 1;
	}
	else{
		print("FAILURE! Floor failed");
		all_tests += 1;
	}
}

void test_ceiling(){
	//scalar
	pd_a3p float64[[1]] value = {15.892356329,5.12974291,7.5009235790235,-52.325623,-12.5002362,-1,873258};
	int64[[1]] control = {16,6,8,-52,-12,-1};
	for(uint i = 0; i < size(value); ++i){
		int64 result = declassify(ceiling(value[i]));
		if(control[i] == result){
			print("SUCCESS");
			all_tests += 1;
			succeeded_tests += 1;
		}
		else{
			print("FAILURE! Floor failed");
			all_tests += 1;
		}
	}
	//vector
	int64[[1]] result = declassify(ceiling(value));
	if(all(control == result)){
		print("SUCCESS");
		all_tests += 1;
		succeeded_tests += 1;
	}
	else{
		print("FAILURE! Floor failed");
		all_tests += 1;
	}
}

void main(){
	print("Additive3pp test: start");

	print("TEST 1: Classify");
	{
		print("uint8");
		uint8 a = 5; pd_a3p uint8 b = classify(a);
		print("SUCCESS!");
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
	}
	{
		print("uint16");
		uint16 a = 5; pd_a3p uint16 b = classify(a); 
		print("SUCCESS!");
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
	}
	{
		print("uint32");
		uint32 a = 5; pd_a3p uint32 b = classify(a); 
		print("SUCCESS!");
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
	}
	{
		print("uint64/uint");
		uint a = 5; pd_a3p uint b = classify(a);
		print("SUCCESS!");
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1; 
	}
	{
		print("int8");
		int8 a = 5; pd_a3p int8 b = classify(a); 
		print("SUCCESS!");
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
	}
	{
		print("int16");
		int16 a = 5; pd_a3p int16 b = classify(a);
		print("SUCCESS!");
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1; 
	}
	{
		print("int32");
		int32 a = 5; pd_a3p int32 b = classify(a); 
		print("SUCCESS!");
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
	}
	{
		print("int64/int");
		int a = 5; pd_a3p int b = classify(a); 
		print("SUCCESS!");
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
	}

	print("TEST 2: Sign");
	{
		print("int8");
		test_sign(0::int8);
	}
	{
		print("int16");
		test_sign(0::int16);
	}
	{
		print("int32");
		test_sign(0::int32);
	}
	{
		print("int64/int");
		test_sign(0::int);
	}
	print("TEST 3: Abs");
	{
		print("int8");
		test_abs(0::int8,0::uint8);
	}
	{
		print("int16");
		test_abs(0::int16,0::uint16);
	}
	{
		print("int32");
		test_abs(0::int32,0::uint32);
	}
	{
		print("int64/int");
		test_abs(0::int,0::uint);
	}
	print("TEST 4: Sum");
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
	print("TEST 5: Sum (2)");
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
	print("TEST 6: Product");
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
	print("TEST 7: Product (2)");
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
	print("TEST 8: Any and All functions");
	{
		print("Any");
		test_any();
	}
	{
		print("All");
		test_all();
	}
	print("TEST 9: True Prefix Length");
	{
		{
			pd_a3p bool[[1]] arr (0);
			uint result = declassify(truePrefixLength(arr));
		}
		for(uint i = 0; i < 5; ++i){
			pd_a3p bool[[1]] arr (10);
			arr = randomize(arr);
			bool[[1]] arr2 = declassify(arr);
			uint result = declassify(truePrefixLength(arr));
			uint control = 0;
			for(uint j = 0; j < size(arr2);++j){
				if(arr2[j]){
					control += 1;
				}
				else{
					break;
				}
			}
			if(control == result){
				succeeded_tests = succeeded_tests + 1;
		 		all_tests = all_tests +1;
		 		print("SUCCESS!");
			}
			else{
				all_tests = all_tests +1;
				print("FAILURE! All failed");
			}
		}
	}
	print("TEST 10: Min");
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
	print("TEST 11: Min (2)");
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
	print("TEST 12: Min (3)");
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
	print("TEST 13: Max");
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
	print("TEST 14: Max (2)");
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
	print("TEST 15: Max (3)");
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
	// Uncomment when SecreC is updated to support these Syscalls
	/*print("TEST 16: Floor");
	{
		test_floor();
	}
	print("TEST 16: Ceiling");
	{
		test_ceiling();
	}*/

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);

    test_report(all_tests, succeeded_tests);
}
