/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module classification_test_scalars;

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

public uint32 all_tests;
public uint32 succeeded_tests;
public bool test_result;

domain pd_a3p additive3pp;

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

template <type T>
bool test_classification(T value){
	public T a; a = value;
	pd_a3p T b; b = a;
	a = declassify(b);
	if(a != value){
		print("FAILURE! Expected value ",value," but got ",a);
		return false;
	}
	else{
		return true;
	}
} 

template <type T, dim N>
bool test_classification(T[[N]] value){
	public T[[N]] a; a = value;
	pd_a3p T[[N]] b; b = a;
	a = declassify(b);
	public bool result;
	result = all(a == value);
	if(result){
		return true;
	}
	else{
		print("FAILURE! Expected value ",arrayToString(value)," but got ",arrayToString(a));
		return false;
	}
}

template <type T>
void test(T pub){
	test_result = test_classification(pub);
	if (test_result) {
	    succeeded_tests = succeeded_tests + 1;
	    all_tests = all_tests +1;
		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
	}
}

template <type T,dim N>
void test(T[[N]] pub){
	test_result = test_classification(pub);
	if (test_result) {
	    succeeded_tests = succeeded_tests + 1;
	    all_tests = all_tests +1;
		print("SUCCESS!");
	}
	else{
		all_tests = all_tests +1;
	}
}

void main(){
	float32 FLOAT32_MAX = 3.4028234e38;
	float64 FLOAT64_MAX = 1.7976931348623157e308;
	float32 FLOAT32_MIN = 1.18e-38;
	float64 FLOAT64_MIN = 2.2250738585072014e-308


	print("classification test: start");

	print("TEST 0a: PUBLIC -> PRIVATE for Float32/64");
	{
		float32 pub = 0.0;
		pd_a3p float32 priv = pub;
	}
	{
		float64 pub = 0.0;
		pd_a3p float64 priv = pub;
	}
	print("TEST 0b: PRIVATE -> PUBLIC for Float32/64");
	{
		pd_a3p float32 priv = 0.0;
		float32 pub = declassify(priv);
	}
	{
		pd_a3p float64 priv = 0.0;
		float64 pub = declassify(priv);
	}
	print("TEST 1: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with 0 ");
 	{
 		print("Float32");
 		public float32 pub = 0.0;
 		test(pub);
 	}
 	{
 		print("Float64");
 		public float64 pub = 0.0;
 		test(pub);
 	}
 	print("TEST 2: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with 1.0");
 	{
 		print("Float32");
 		public float32 pub = 1.0;
 		test(pub);
 	}
 	{
 		print("Float64");
 		public float64 pub = 1.0;
 		test(pub);
 	}
 	print("TEST 3: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with -1.0");
 	{
 		print("Float32");
 		public float32 pub = -1.0;
 		test(pub);
 	}
 	{
 		print("Float64");
 		public float64 pub = -1.0;
 		test(pub);
 	}
 	print("TEST 4: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with MAX positive values");
 	{
 		print("Float32 : ", FLOAT32_MAX);
 		test(FLOAT32_MAX);
 	}
 	{
 		print("Float64 : ", FLOAT64_MAX);
 		test(FLOAT64_MAX);
 	}
 	print("TEST 5: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with MAX negative values");
 	{
 		print("Float32 : ", -FLOAT32_MAX);
 		test(-FLOAT32_MAX);
 	}
 	{
 		print("Float64 : ", -FLOAT64_MAX);
 		test(-FLOAT64_MAX);
 	}
 	print("TEST 6: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with MIN positive values");
 	{
 		print("Float32 : ", FLOAT32_MIN);
 		test(FLOAT32_MIN);
 	}
 	{
 		print("Float64 : ", FLOAT64_MIN);
 		test(FLOAT64_MIN);
 	}
 	print("TEST 7: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with MIN negative values");
	{
 		print("Float32 : ", -FLOAT32_MIN);
 		test(-FLOAT32_MIN);
 	}
 	{
 		print("Float64 : ", -FLOAT64_MIN);
 		test(-FLOAT64_MIN);
 	}
 	print("TEST 8: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with random values");
 	{
 		print("Float32");
 		public float32 pub = 162874612958512.981269182562198642198623;
 		test(pub);
 		pub = 1982651298573509437643.598327659803275819256127984621748;
 		test(pub);
 		pub = 59036783265903267329856195.7832658723657128561245872457583265823;
 		test(pub);
 		pub = 43298537829532953.87326587325632875632874215428714;
 		test(pub);
 		pub = -29872165932578329573285.89832659823657328563;
 		test(pub);
 		pub = -6732854632859328563298.648732458128742198;
 		test(pub);
 		pub = -9853467894363275093275893247325.8932659873265832754;
 		test(pub);
 		pub = random_float(0::float32);
 		test(pub);
 		pub = random_float(0::float32);
 		test(pub);
 		pub = random_float(0::float32);
 		test(pub);
 		pub = random_float(0::float32);
 		test(pub);
 		pub = random_float(0::float32);
 		test(pub);
 		pub = random_float(0::float32);
 		test(pub);
 	}
 	{
 		print("Float64");
 		public float64 pub = 162874612923523532558512.981269182562643643634198642198623;
 		test(pub);
 		pub = 1982632423423551298573509437643.59832765980327581918264219849256127984621748;
 		test(pub);
 		pub = 5903678326590332512759267329856195.7832658723657128566219841245872457583265823;
 		test(pub);
 		pub = 43298537216985416274832593257829532953.873265873256328329807512475632874215428714;
 		test(pub);
 		pub = -29872121984721503265932578329573285.89832659329759832412823657328563;
 		test(pub);
 		pub = -32047325906732854632859332532528563298.64873245813253253228742198;
 		test(pub);
 		pub = -985346789436398216598326578356275093275893247325.89326598798127492143265832754;
 		test(pub);
 		pub = random_float(0::float64);
 		test(pub);
 		pub = random_float(0::float64);
 		test(pub);
 		pub = random_float(0::float64);
 		test(pub);
 		pub = random_float(0::float64);
 		test(pub);
 		pub = random_float(0::float64);
 		test(pub);
 		pub = random_float(0::float64);
 		test(pub);
 	}
 	print("TEST 9: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with -1/3 and +1/3");
 	{
 		print("Float32");
 		public float32 pub = 1/3;
 		test(pub);
 		pub = -1/3;
 		test(pub);
 	}
 	{
 		print("Float64");
 		public float64 pub = 1/3;
 		test(pub);
 		pub = -1/3;
 		test(pub);
 	}

 	print("TEST 10: PUBLIC -> PRIVATE for Float32/64 vectors");
	{
		float32[[1]] pub (15)= 0.0;
		pd_a3p float32[[1]] priv = pub;
	}
	{
		float64[[1]] pub (15)= 0.0;
		pd_a3p float64[[1]] priv = pub;
	}
	print("TEST 11: PRIVATE -> PUBLIC for Float32/64 vectors");
	{
		pd_a3p float32[[1]] priv (15)= 0.0;
		float32[[1]] pub = declassify(priv);
	}
	{
		pd_a3p float64[[1]] priv (15)= 0.0;
		float64[[1]] pub = declassify(priv);
	}
	print("TEST 12: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with 0 vectors ");
 	{
 		print("Float32");
 		public float32[[1]] pub (15)= 0.0;
 		test(pub);
 	}
 	{
 		print("Float64");
 		public float64[[1]] pub (15)= 0.0;
 		test(pub);
 	}
 	print("TEST 13: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with 1.0 vectors");
 	{
 		print("Float32");
 		public float32[[1]] pub (15)= 1.0;
 		test(pub);
 	}
 	{
 		print("Float64");
 		public float64[[1]] pub (15)= 1.0;
 		test(pub);
 	}
 	print("TEST 14: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with -1.0 vectors");
 	{
 		print("Float32");
 		public float32[[1]] pub (15)= -1.0;
 		test(pub);
 	}
 	{
 		print("Float64");
 		public float64[[1]] pub (15)= -1.0;
 		test(pub);
 	}

 	print("TEST 15: PUBLIC -> PRIVATE -> PUBLIC conversion with Float32/64 MAX/MIN values over 1-10 element vectors");
 	for(uint i = 1; i < 11; ++i){
		{	
			print("Float32: ", i ," element vector (MAX VALUE)");
			float32[[1]] pub (i) = FLOAT32_MAX;
			test(pub);
			print("Float32: ", i ," element vector (-MAX VALUE)");
			pub = -FLOAT32_MAX;
			test(pub);
			print("Float32: ", i ," element vector (MIN VALUE)");
			float32[[1]] pub2 (i) = FLOAT32_MIN;
			test(pub2);
			print("Float32: ", i ," element vector (-MIN VALUE)");
			pub2 = -FLOAT32_MIN;
			test(pub2);
		}
		{
			print("Float64: ", i ," element vector (MAX VALUE)");
			float64[[1]] pub (i) = FLOAT64_MAX;
			test(pub);
			print("Float64: ", i ," element vector (-MAX VALUE)");
			pub = -FLOAT64_MAX;
			test(pub);
			print("Float64: ", i ," element vector (MIN VALUE)");
			float64[[1]] pub2 (i) = FLOAT64_MIN;
			test(pub2);
			print("Float64: ", i ," element vector (-MIN VALUE)");
			pub2 = -FLOAT64_MIN;
			test(pub2);
		}
 	}

 	print("TEST 8: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with random vectors");
 	{
 		print("Float32");
 		public float32[[1]] pub (15)= 162874612958512.981269182562198642198623;
 		test(pub);
 		pub = 1982651298573509437643.598327659803275819256127984621748;
 		test(pub);
 		pub = 59036783265903267329856195.7832658723657128561245872457583265823;
 		test(pub);
 		pub = 43298537829532953.87326587325632875632874215428714;
 		test(pub);
 		pub = -29872165932578329573285.89832659823657328563;
 		test(pub);
 		pub = -6732854632859328563298.648732458128742198;
 		test(pub);
 		pub = -9853467894363275093275893247325.8932659873265832754;
 		test(pub);
 		pd_a3p float32[[1]] pub2 (10);
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 	}
 	{
 		print("Float64");
 		public float64[[1]] pub (15)= 162874612923523532558512.981269182562643643634198642198623;
 		test(pub);
 		pub = 1982632423423551298573509437643.59832765980327581918264219849256127984621748;
 		test(pub);
 		pub = 5903678326590332512759267329856195.7832658723657128566219841245872457583265823;
 		test(pub);
 		pub = 43298537216985416274832593257829532953.873265873256328329807512475632874215428714;
 		test(pub);
 		pub = -29872121984721503265932578329573285.89832659329759832412823657328563;
 		test(pub);
 		pub = -32047325906732854632859332532528563298.64873245813253253228742198;
 		test(pub);
 		pub = -985346789436398216598326578356275093275893247325.89326598798127492143265832754;
 		test(pub);
 		pd_a3p float64[[1]] pub2 (10);
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 		pub2 = random(pub2);
 		test(declassify(pub2));
 	}
 	print("TEST 9: PUBLIC -> PRIVATE -> PUBLIC for Float32/64 with -1/3 and +1/3 vectors");
 	{
 		print("Float32");
 		public float32[[1]] pub (15)= 1/3;
 		test(pub);
 		pub = -1/3;
 		test(pub);
 	}
 	{
 		print("Float64");
 		public float64[[1]] pub (15)= 1/3;
 		test(pub);
 		pub = -1/3;
 		test(pub);
 	}



 	print("Test finished!");
 	print("Succeeded tests: ", succeeded_tests);
 	print("Failed tests: ", all_tests - succeeded_tests);
 }