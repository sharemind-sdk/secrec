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

domain pd_a3p additive3pp;

public uint32 all_tests;
public uint32 succeeded_tests;


template<domain D:additive3pp,type T>
void test_min(D T data){
	pd_a3p T[[1]] temp (25);
	temp = randomize(temp);
	pd_a3p T[[1]] vec = declassify(temp);
	pd_a3p T result = min(temp);
	pd_a3p T control = 0;
	for(uint i = 0; i < size(vec);++i){
		if(i == 0){
			control = vec[i];
		}
		else{
			if(declassify(vec[i]) < declassify(control)){
				control = vec[i];
			}
		}
	} 
	if(declassify(control) == declassify(result)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Min failed");
	}
}

template<domain D:additive3pp,type T>
void test_min2(D T data){
	{
		pd_a3p T[[1]] temp (0);
		pd_a3p T[[1]] temp2 (0);
		pd_a3p T[[1]] result = min(temp,temp2);
	}
	pd_a3p T[[1]] temp (10);
	pd_a3p T[[1]] temp2 (10);
	temp = randomize(temp);
	temp2 = randomize(temp2);
	pd_a3p T[[1]] vec = temp;
	pd_a3p T[[1]] vec2 = temp2;
	pd_a3p T[[1]] result = min(temp,temp2);
	pd_a3p T[[1]] control (10) = 0;
	for(uint i = 0; i < size(vec);++i){
		if(declassify(vec[i]) <= declassify(vec2[i])){
			control[i] = vec[i]; 
		}
		else{
			control[i] = vec2[i];
		}
	} 
	if(all(declassify(control) == declassify(result))){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Min failed");
	}
}

template<domain D:additive3pp,type T>
void test_max(D T data){
	pd_a3p T[[1]] temp (25);
	temp = randomize(temp);
	pd_a3p T[[1]] vec = temp;
	pd_a3p T result = max(temp);
	pd_a3p T control = 0;
	for(uint i = 0; i < size(vec);++i){
		if(declassify(vec[i]) > declassify(control)){
			control = vec[i];
		}
	} 
	if(declassify(control) == declassify(result)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Max failed");
	}
}

template<domain D:additive3pp,type T>
void test_max2(D T data){
	{
		pd_a3p T[[1]] temp (0);
		pd_a3p T[[1]] temp2 (0);
		pd_a3p T[[1]] result = max(temp,temp2);
	}
	pd_a3p T[[1]] temp (10);
	pd_a3p T[[1]] temp2 (10);
	temp = randomize(temp);
	temp2 = randomize(temp2);
	pd_a3p T[[1]] vec = temp;
	pd_a3p T[[1]] vec2 = temp2;
	pd_a3p T[[1]] result = max(temp,temp2);
	pd_a3p T[[1]] control (10) = 0;
	for(uint i = 0; i < size(vec);++i){
		if(declassify(vec[i]) >= declassify(vec2[i])){
			control[i] = vec[i]; 
		}
		else{
			control[i] = vec2[i];
		}
	}
	if(all(declassify(control) == declassify(result))){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
		print("FAILURE! Max failed");
	}
}

template<domain D:additive3pp,type T>
void test_reshare(D T data){
	pd_a3p T scalar = 0;
	scalar = randomize(scalar);
	pd_a3p T scalar2 = reshare(reshare(scalar));
	if(declassify(scalar) == declassify(scalar2)){
		print("SUCCESS!");
		all_tests += 1;
		succeeded_tests += 1;
	}
	else{
		print("FAILURE!");
		all_tests += 1;
	}
	{
		pd_a3p T[[1]] vector (0);
		pd_a3p T[[1]] result = reshare(reshare(vector));
	}
	pd_a3p T[[1]] vector (15) = 0;
	vector = randomize(vector);
	pd_a3p T[[1]] vector2 = reshare(reshare(vector));
	if(all(declassify(vector) == declassify(vector2))){
		print("SUCCESS!");
		all_tests += 1;
		succeeded_tests += 1;
	}
	else{
		print("FAILURE!");
		all_tests += 1;
	}

}

void main(){
	print("Additive3pp test: start");

	print("TEST 1: Min");
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		test_min(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		test_min(data);
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 data = 0;
		test_min(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		test_min(data);
	}
	print("TEST 2: Min (2)");
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		test_min2(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		test_min2(data);
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 data = 0;
		test_min2(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		test_min2(data);
	}
	print("TEST 3: Max");
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		test_max(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		test_max(data);
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 data = 0;
		test_max(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		test_max(data);
	}
	print("TEST 4: Max (2)");
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		test_max2(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		test_max2(data);
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 data = 0;
		test_max2(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		test_max2(data);
	}
	print("TEST 5: Resharing");
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		test_reshare(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		test_reshare(data);
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 data = 0;
		test_reshare(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		test_reshare(data);
	}


	


	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
} 
