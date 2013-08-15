/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module Arithmetic_test_scalar;

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

void Success(){
	succeeded_tests = succeeded_tests + 1;
	all_tests = all_tests +1;
	print("SUCCESS!");
}

template<type T,domain D: additive3pp>
void Failure(string s, D T c){
	print("FAILURE! ",s);
	print("got: ",declassify(c));
	all_tests = all_tests +1;
}
template<type T>
void Failure(string s,T c){
	print("FAILURE! ",s);
	print("got: ", c);
	all_tests = all_tests +1;
}

template<type T>
void test_addition_random(T data){
	T[[1]] in = {1,1000,1000000,-1,-1000,-1000000};
	T[[1]] control = {0,999,999999,-2,-1001,-1000001,1,1000,1000000,-1,-1000,-1000000,2,1001,1000001,0,-999,-999999};
	bool result = true;
	uint temp = 0;
	uint cur_pos = 0;
	pd_a3p T a;pd_a3p T b;pd_a3p T c;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			a = in[j]; b = (T)i;
			c = a + b;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
			c = b + a;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	T d;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			a = in[j]; d = (T)i;
			c = a + d;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
			c = d + a;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			d = in[j]; a = (T)i;
			c = d + a;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
			c = a + d;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	T e; T f;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			d = in[j]; e = (T)i;
			f = d + e;
			if(f != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", f);
				result = false;
			}
			f = e + d;
			if(f != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", f);
				result = false;
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1;}
}

template<type T>
void test_subtraction_random(T data){
	T[[1]] in = {1,1000,1000000,-1,-1000,-1000000};
	T[[1]] control = {2,1001,1000001,0,-999,-999999,1,1000,1000000,-1,-1000,-1000000,0,999,999999,-2,-1001,-1000001};
	bool result = true;
	uint temp = 0;
	uint cur_pos = 0;
	pd_a3p T a;pd_a3p T b;pd_a3p T c;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			a = in[j]; b = (T)i;
			c = a - b;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	T d;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			a = in[j]; d = (T)i;
			c = a - d;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			d = in[j]; a = (T)i;
			c = d - a;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	T e; T f;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			d = in[j]; e = (T)i;
			f = d - e;
			if(f != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", f);
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
}


template<type T>
void test_multiplication_random(T data){
	T[[1]] in = {1,1000,1000000,-1,-1000,-1000000};
	T[[1]] control = {-1,-1000,-1000000,1,1000,1000000,0,0,0,0,0,0,1,1000,1000000,-1,-1000,-1000000};
	bool result = true;
	uint temp = 0;
	uint cur_pos = 0;
	pd_a3p T a;pd_a3p T b;pd_a3p T c;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			a = in[j]; b = (T)i;
			c = a * b;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
			c = b * a;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	T d;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			a = in[j]; d = (T)i;
			c = a * d;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
			c = d * a;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			d = in[j]; a = (T)i;
			c = d * a;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
			c = a * d;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
				result = false;
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	T e; T f;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			d = in[j]; e = (T)i;
			f = d * e;
			if(f != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", f);
				result = false;
			}
			f = e * d;
			if(f != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", f);
				result = false;
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1;}
}

template<type T>
void test_division_random(T data){
	T[[1]] in = {1,1000,1000000,-1,-1000,-1000000};
	T[[1]] control = {-1,-0.001,-0.000001,1,0.001,0.000001,0,0,0,0,0,0,1,0.001,0.000001,-1,-0.001,-0.000001};
	bool result = true;
	uint temp = 0;
	uint cur_pos = 0;
	pd_a3p T a;pd_a3p T b;pd_a3p T c;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			a = in[j]; b = (T)i;
			c = b / a;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	T d;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			a = in[j]; d = (T)i;
			c = d / a;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			d = in[j]; a = (T)i;
			c = a / d;
			if(declassify(c) != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", declassify(c));
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
	result = true;
	temp = 0;cur_pos = 0;
	T e; T f;
	for(int i = -1; i < 2;++i){
		for(uint j = 0; j < 6; ++j){
			cur_pos = (temp*6) + j;
			d = in[j]; e = (T)i;
			f = e / d;
			if(f != control[cur_pos]){
				print("FAILURE! Expected: ", control[cur_pos]);
				print("Got: ", f);
			}
		}
		temp += 1;
	}
	if(result){ print("SUCCESS"); all_tests+=1; succeeded_tests +=1; }
}




void main(){

	print("Arithmetic test: start");

	print("TEST 1: Addition with two public values");
	{
		print("float32");
		float32 a = 15; float32 b = 174; float32 c = a+b;
		if(c == 189){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("float64");
		float64 a = 15; float64 b = 174; float64 c = a+b;
		if(c == 189){Success();}else{Failure("Expected 189",c);}
	}
	print("TEST 2: Addition with two private values");
	{
		print("float32");
		pd_a3p float32 a = 15; pd_a3p float32 b = 174; pd_a3p float32 c = a+b;
		if(declassify(c) == 189){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 175; pd_a3p float64 b = 45876; pd_a3p float64 c = a+b;
		if(declassify(c) == 46051){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 3: Addition with one private one public value");
	{
		print("float32");
		pd_a3p float32 a = 15; float32 b = 174; pd_a3p float32 c = a+b;
		if(declassify(c) == 189){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 175; float64 b = 45876; pd_a3p float64 c = a+b;
		if(declassify(c) == 46051){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 4: Addition with one private one public value(2)");
	{
		print("float32");
		pd_a3p float32 a = 15; float32 b = 174; pd_a3p float32 c = b+a;
		if(declassify(c) == 189){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 175; float64 b = 45876; pd_a3p float64 c = b+a;
		if(declassify(c) == 46051){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 5: Addition with important values.All combinations of public and private");
	{
		print("float32");
		test_addition_random(0::float32);
	}
	{
		print("float64");
		test_addition_random(0::float64);
	}








	print("TEST 6: Subtraction with two public values");
	{
		print("float32");
		float32 a = 15; float32 b = 174; float32 c = b-a;
		if(c == 159){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("float64");
		float64 a = 175; float64 b = 45876; float64 c = b-a;
		if(c == 45701){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 7: Subtraction with two private values");
	{
		print("float32");
		pd_a3p float32 a = 15; pd_a3p float32 b = 174; pd_a3p float32 c = b-a;
		if(declassify(c) == 159){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 175; pd_a3p float64 b = 45876; pd_a3p float64 c = b-a;
		if(declassify(c) == 45701){Success();}else{Failure("Expected 45701",c);}
	}
	print("TEST 8: Subtraction with one private one public value");
	{
		print("float32");
		pd_a3p float32 a = 15; float32 b = 174; pd_a3p float32 c = b-a;
		if(declassify(c) == 159){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 175; float64 b = 45876; pd_a3p float64 c = b-a;
		if(declassify(c) == 45701){Success();}else{Failure("Expected 45701",c);}
	}
	print("TEST 9: Subtraction with one private one public value(2)");
	{
		print("float32");
		pd_a3p float32 a = 174; float32 b = 15; pd_a3p float32 c = a-b;
		if(declassify(c) == 159){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 45876; float64 b = 175; pd_a3p float64 c = a-b;
		if(declassify(c) == 45701){Success();}else{Failure("Expected 45701",c);}
	}
	print("TEST 10: Subtraction with private-private important values");
	{
		print("float32");
		test_subtraction_random(0::float32);
	}
	{
		print("float64");
		test_subtraction_random(0::float64);
	}








	print("TEST 11: Multiplication with two public values");
	{
		print("float32");
		float32 a = 15; float32 b = 12; float32 c = a*b;
		if(c == 180){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("float64");
		float64 a = 175; float64 b = 139; float64 c = a*b;
		if(c == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	print("TEST 12: Multiplication with two private values");
	{
		print("float32");
		pd_a3p float32 a = 15; pd_a3p float32 b = 12; pd_a3p float32 c = a*b;
		if(declassify(c) == 180){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 175; pd_a3p float64 b = 139; pd_a3p float64 c = a*b;
		if(declassify(c) == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	print("TEST 13: Multiplication with one private one public value");
	{
		print("float32");
		pd_a3p float32 a = 15; float32 b = 12; pd_a3p float32 c = a*b;
		if(declassify(c) == 180){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 175; float64 b = 139; pd_a3p float64 c = a*b;
		if(declassify(c) == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	print("TEST 14: Multiplication with one private one public value(2)");
	{
		print("float32");
		pd_a3p float32 a = 15; float32 b = 12; pd_a3p float32 c = b*a;
		if(declassify(c) == 180){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 175; float64 b = 139; pd_a3p float64 c = b*a;
		if(declassify(c) == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	print("TEST 15: Multiplication with important values.All combinations of public and private");
	{
		print("float32");
		test_multiplication_random(0::float32);
	}
	{
		print("float64");
		test_multiplication_random(0::float64);
	}









	print("TEST 16: Division with two public values");
	{
		print("float32");
		float32 a = 15; float32 b = 174; float32 c = b/a;
		if(c == 11.6){Success();}else{Failure("Expected 11.6",c);}
	}
	{
		print("float64");
		float64 a = 180; float64 b = 45900; float64 c = b/a;
		if(c == 255){Success();}else{Failure("Expected 255",c);}
	}
	print("TEST 17: Division with two private values");
	{
		print("float32");
		pd_a3p float32 a = 15; pd_a3p float32 b = 174; pd_a3p float32 c = b/a;
		if(declassify(c) == 11.6){Success();}else{Failure("Expected 11.6",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 180; pd_a3p float64 b = 45900; pd_a3p float64 c = b/a;
		if(declassify(c) == 255){Success();}else{Failure("Expected 255",c);}
	}
	print("TEST 18: Division with one private one public value");
	{
		print("float32");
		pd_a3p float32 a = 15; float32 b = 174; pd_a3p float32 c = b/a;
		if(declassify(c) == 11.6){Success();}else{Failure("Expected 11.6",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 180; float64 b = 45900; pd_a3p float64 c = b/a;
		if(declassify(c) == 255){Success();}else{Failure("Expected 255",c);}
	}
	print("TEST 19: Division with one private one public value(2)");
	{
		print("float32");
		pd_a3p float32 a = 180; float32 b = 15; pd_a3p float32 c = a/b;
		if(declassify(c) == 12){Success();}else{Failure("Expected 12",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 45900; float64 b = 180; pd_a3p float64 c = a/b;
		if(declassify(c) == 255){Success();}else{Failure("Expected 255",c);}
	}
	print("TEST 20: 0 divided with random public values");
	{
		print("float32");
		float32 a = 15; float32 b = 0; float32 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("float64");
		float64 a = 175; float64 b = 0; float64 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	print("TEST 21: private 0 divided with random public values");
	{
		print("float32");
		float32 a = 2398; pd_a3p float32 b = 0;pd_a3p float32 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("float64");
		float64 a = 9083275; pd_a3p float64 b = 0;pd_a3p float64 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	print("TEST 21a: Division of private 0 with public 2^n");
	{
		print("float32");
		pd_a3p float32 a = 0;
		float32 b = 2;
		pd_a3p float32 c;
		for(uint i = 0; i < 10; ++i){
			c = a/b;
			if(declassify(c) != 0){
				print("FAILURE! Expected: 0 Got: ", declassify(c));
			}
			else{
				print("SUCCESS!");
			}
			b *= 2;
		}
	}
	{
		print("float64");
		pd_a3p float64 a = 0;
		float64 b = 2;
		pd_a3p float64 c;
		for(uint i = 0; i < 10; ++i){
			c = a/b;
			if(declassify(c) != 0){
				print("FAILURE! Expected: 0 Got: ", declassify(c));
			}
			else{
				print("SUCCESS!");
			}
			b *= 2;
		}
	}
	print("TEST 22: A/A = 1 with all public types");
	{
		print("float32");
		float32 a = 174; float32 b = 174; float32 c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	{
		print("float64");
		float64 a = 45876; float64 b = 45876; float64 c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	print("TEST 23: Division accuracy public");
	{
		print("float32");
		float32 a = 645; float32 b = 40; float32 c = a/b;
		if(c == 16.125){Success();}else{Failure("Expected 16.125",c);}
	}
	{
		print("float64");
		float64 a = 645; float64 b = 40; float64 c = a/b;
		if(c == 16.125){Success();}else{Failure("Expected 16.125",c);}
	}
	print("TEST 24: Division accuracy private");
	{
		print("float32");
		pd_a3p float32 a = 645; pd_a3p float32 b = 40; pd_a3p float32 c = a/b;
		if(declassify(c) == 16.125){Success();}else{Failure("Expected 16.125",c);}
	}
	{
		print("float64");
		pd_a3p float64 a = 645; pd_a3p float64 b = 40; pd_a3p float64 c = a/b;
		if(declassify(c) == 16.125){Success();}else{Failure("Expected 16.125",c);}
	}
	print("TEST 25: Division with important values.All combinations of public and private");
	{
		print("float32");
		test_division_random(0::float32);
	}
	{
		print("float64");
		test_division_random(0::float64);
	}









	print("TEST 26: Operation priorities : Multiplication over addition");
	{
		print("float32");
		float32 a = 5; float32 b = 20; float32 c = 45;
		float32 d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",d);}
	}
	{
		print("float64");
		float64 a = 5; float64 b = 20; float64 c = 45;
		float64 d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",d);}
	}
	print("TEST 27: Operation priorities : Parentheses over multiplication");
	{
		print("float32");
		float32 a = 5; float32 b = 5; float32 c = 20;
		float32 d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	{
		print("float64");
		float64 a = 5; float64 b = 5; float64 c = 20;
		float64 d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	print("TEST 28: Operation priorities : Division over addition and subtraction");
	{
		print("float32");
		float32 a = 5; float32 b = 5; float32 c = 20; float32 d = 5;
		float32 e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}
	{
		print("float64");
		float64 a = 5; float64 b = 5; float64 c = 20; float64 d = 5;
		float64 e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}
	print("TEST 29: public boolean negotiation (!)");
	{
		print("float32");
		float32 a = 25;
		float32 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",true);}
	}
	{
		print("float64");
		float64 a = 25;
		float64 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",true);}
	}



	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}