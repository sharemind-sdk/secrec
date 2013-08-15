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

template <type T>
void test_0a(T pub){
 	pd_a3p T priv;
 	priv = pub;
 	succeeded_tests = succeeded_tests + 1;
 	all_tests = all_tests +1;
 	print("SUCCESS!");
}

template <domain D : additive3pp, type T>
void test_0b(D T priv){
	public T pub;
 	pub = declassify(priv);
 	succeeded_tests = succeeded_tests + 1;
 	all_tests = all_tests +1;
 	print("SUCCESS!");
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

template <type T>
void rand_test(T pub, uint32 nr){
	public bool result = true;
 	for(public uint32 i = 0; i < nr; ++i){
 		pd_a3p T[[1]] a (1); 
 		a = randomize(a);
 		pd_a3p T priv = a[0];
 		test_result = test_classification(declassify(priv));
 		result = test_result && result;
 	}
 	if(result){
 		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
 	}
 	else{
 		all_tests = all_tests +1;
 	}
}

template<domain D : additive3pp ,type T, type T2>
void xor_test(D T priv, T2 pub){
	if(declassify(priv) != pub){
 		all_tests = all_tests + 1;
 		print("FAILURE! Expected value ",pub," but got ",declassify(priv));
 	}
 	else{
 		print("SUCCESS!");
 		all_tests = all_tests + 1;
 		succeeded_tests = succeeded_tests + 1;
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

 	print("Classification test: start");

 	print("TEST 0a: PUBLIC -> PRIVATE conversions throws no errors");
 	{
 		print("boolean");
 		public bool pub = true;
 		test_0a(pub);
 	}
 	{
 		print("uint8");
 		public uint8 pub = 1;
 		test_0a(pub);
 	}
 	{
 		print("uint16");
 		public uint16 pub = 1;
 		test_0a(pub);
 	}
 	{
 		print("uint32");
 		public uint32 pub = 1;
 		test_0a(pub);
 	}
 	{
 		print("uint64");
 		public uint pub = 1;
 		test_0a(pub);
 	}
 	{
 		print("int8");
 		public int8 pub = 1;
 		test_0a(pub);
 	}
 	{
 		print("int16");
 		public int16 pub = 1;
 		test_0a(pub);
 	}
 	{
 		print("int32");
 		public int32 pub = 1;
 		test_0a(pub);
 	}
 	{
 		print("int64/int");
 		public int pub = 1;
 		test_0a(pub);
 	}
 	print("TEST 0b: PRIVATE -> PUBLIC conversion throws no errors");
 	{
 		print("boolean");
 		pd_a3p bool priv = false;
 		test_0b(priv);
 	}
 	{
 		print("uint8");
 		pd_a3p uint8 priv = 0;
 		test_0b(priv);
 	}
 	{
 		print("uint16");
 		pd_a3p uint16 priv = 0;
 		test_0b(priv);
 	}
 	{
 		print("uint32");
 		pd_a3p uint32 priv = 0;
 		test_0b(priv);
 	}
 	{
 		print("uint64/uint");
 		pd_a3p uint priv = 0;
 		test_0b(priv);
 	}
 	{
 		print("int8");
 		pd_a3p int8 priv = 0;
 		test_0b(priv);
 	}
 	{
 		print("int16");
 		pd_a3p int16 priv = 0;
 		test_0b(priv);
 	}
 	{
 		print("int32");
 		pd_a3p int32 priv = 0;
 		test_0b(priv);
 	}
 	{
 		print("int64/int");
 		pd_a3p int priv = 0;
 		test_0b(priv);
 	}
 	{
 		print("xor_uint8");
 		pd_a3p xor_uint8 priv = 0;
 		public uint8 pub = 0;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint16");
 		pd_a3p xor_uint16 priv = 0;
 		public uint16 pub = 0;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint32");
 		pd_a3p xor_uint32 priv = 0;
 		public uint32 pub = 0;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint64");
 		pd_a3p xor_uint64 priv = 0;
 		public uint64 pub = 0;
 		xor_test(priv,pub);
 	}
 	print("TEST 1: PUBLIC -> PRIVATE -> PUBLIC conversion with (0)");
	{
 		print("boolean");
 		public bool pub = false;
	 	test(pub);
    } 	
 	{
 		print("uint8");
 		public uint8 pub = 0;
	 	test(pub);
    }
 	{
 		print("uint16");
 		public uint16 pub = 0;
 		test(pub);
 	}
 	{
 		print("uint32");
 		public uint32 pub = 0;
 		test(pub);
 	}
 	{
 		print("uint64/uint");
 		public uint pub = 0;
 		test(pub);
 	}
 	{
 		print("int8");
 		public int8 pub = 0;
 		test(pub);
 	}
 	{
 		print("int16");
 		public int16 pub = 0;
 		test(pub);
 	}
 	{
 		print("int32");
 		public int32 pub = 0;
 		test(pub);
 	}
 	{
 		print("int64/int");
 		public int64 pub = 0;
 		test(pub);
 	}
 	print("TEST 2: PUBLIC -> PRIVATE -> PUBLIC conversion with (1)");
 	{
 		print("boolean");
 		public bool pub = true;
 		test(pub);
    }	
 	{
 		print("uint8");
 		public uint8 pub = 1;
 		test(pub);
    }
 	{
 		print("uint16");
 		public uint16 pub = 1;
 		test(pub);
 	}
 	{
 		print("uint32");
 		public uint32 pub = 1;
 		test(pub);
 	}
 	{
 		print("uint64/uint");
 		public uint64 pub = 1;
 		test(pub);
 	}
 	{
 		print("int8");
 		public int8 pub = 1;
 		test(pub);
    }
    {
 		print("int16");
 		public int16 pub = 1;
 		test(pub);
    }
    {
 		print("int32");
 		public int32 pub = 1;
 		test(pub);
    }
    {
 		print("int/int64");
 		public int pub = 1;
 		test(pub);
    }
    {
 		print("xor_uint8");
 		pd_a3p xor_uint8 priv = 1;
 		public uint8 pub = 1;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint16");
 		pd_a3p xor_uint16 priv = 1;
 		public uint16 pub = 1;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint32");
 		pd_a3p xor_uint32 priv = 1;
 		public uint32 pub = 1;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint64");
 		pd_a3p xor_uint64 priv = 1;
 		public uint64 pub = 1;
 		xor_test(priv,pub);
 	}
    print("TEST 3: PUBLIC -> PRIVATE -> PUBLIC conversion with (-1)" );
	{
 		print("int8");
 		public int8 pub = -1;
 		test(pub);
    }
    {
 		print("int16");
 		public int16 pub = -1;
 		test(pub);
    }
    {
 		print("int32");
 		public int32 pub = -1;
 		test(pub);
    }
    {
 		print("int/int64");
 		public int pub = -1;
 		test(pub);
    }
 	print("TEST 4: PUBLIC -> PRIVATE -> PUBLIC with MAX-1 values");
 	{
 		print("uint8 : ",UINT8_MAX -1);
	    test((UINT8_MAX-1));
    }
 	{
 		print("uint16 : ",UINT16_MAX -1);
 		test((UINT16_MAX-1));
 	}
 	{
 		print("uint32 : ",UINT32_MAX -1);
 		test((UINT32_MAX-1));
 	}
 	{
 		print("uint64/uint : ",UINT64_MAX -1);
 		test((UINT64_MAX-1));
 	}
 	{
 		print("int8 : ",INT8_MAX-1);
 		test((UINT8_MAX-1));
 	}
 	{
 		print("int16 : ",UINT16_MAX -1);
 		test((UINT16_MAX-1));
 	}
 	{
 		print("int32 : ",INT32_MAX -1);
 		test((UINT32_MAX-1));
 	}
 	{
 		print("int64/int : ",INT64_MAX -1);
 		test((INT64_MAX-1));
 	}
 	{
 		print("xor_uint8");
 		pd_a3p xor_uint8 priv = UINT8_MAX-1;
 		public uint8 pub = UINT8_MAX-1;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint16");
 		pd_a3p xor_uint16 priv = UINT16_MAX-1;
 		public uint16 pub = UINT16_MAX-1;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint32");
 		pd_a3p xor_uint32 priv = UINT32_MAX-1;
 		public uint32 pub = UINT32_MAX-1;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint64");
 		pd_a3p xor_uint64 priv = UINT64_MAX-1;
 		public uint64 pub = UINT64_MAX-1;
 		xor_test(priv,pub);
 	}
 	print("TEST 5: PUBLIC -> PRIVATE -> PUBLIC with MAX values");
 	{
 		print("uint8 : ",UINT8_MAX);
	 	test((UINT8_MAX));
    }
 	{
 		print("uint16 : ",UINT16_MAX);
 		test((UINT16_MAX));
 	}
 	{
 		print("uint32 : ",UINT32_MAX);
 		test((UINT32_MAX));
 	}
 	{
 		print("uint64/uint : ",UINT64_MAX);
 		test((UINT64_MAX));
 	}
 	{
 		print("int8 : ",INT8_MAX);
 		test((INT8_MAX));
 	}
 	{
 		print("int16 : ",INT16_MAX);
 		test((INT16_MAX));
 	}
 	{
 		print("int32 : ",INT32_MAX);
 		test((INT32_MAX));
 	}
 	{
 		print("int64/int : ",INT64_MAX);
 		test((INT64_MAX));
 	}
 	{
 		print("xor_uint8");
 		pd_a3p xor_uint8 priv = UINT8_MAX;
 		public uint8 pub = UINT8_MAX;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint16");
 		pd_a3p xor_uint16 priv = UINT16_MAX;
 		public uint16 pub = UINT16_MAX;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint32");
 		pd_a3p xor_uint32 priv = UINT32_MAX;
 		public uint32 pub = UINT32_MAX;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint64");
 		pd_a3p xor_uint64 priv = UINT64_MAX;
 		public uint64 pub = UINT64_MAX;
 		xor_test(priv,pub);
 	}
 	print("TEST 6: PUBLIC -> PRIVATE -> PUBLIC with MIN+1 values");
 	{
 		print("int8 : ",INT8_MIN+1);
 		test((INT8_MIN+1));
 	}
 	{
 		print("int16 : ",INT16_MIN+1);
 		test((INT16_MIN+1));
 	}
 	{
 		print("int32 : ",INT32_MIN+1);
 		test((INT32_MIN+1));
 	}
 	{
 		print("int64/int : ",INT64_MIN+1);
 		test((INT64_MIN+1));
 	}
 	print("TEST 7: PUBLIC -> PRIVATE -> PUBLIC with MIN values");
 	{
 		print("int8 : ",INT8_MIN);
 		test((INT8_MIN));
 	}
 	{
 		print("int16 : ",INT16_MIN);
 		test((INT16_MIN));
 	}
 	{
 		print("int32 : ",INT32_MIN);
 		test((INT32_MIN));
 	}
 	{
 		print("int64/int : ",INT64_MIN);
 		test((INT64_MIN));
 	}

 	public uint32 random_tests = 10; // change number of tests the computer does
 	print("TEST 8: PUBLIC -> PRIVATE -> PUBLIC with ", random_tests," random values");
 	{
 		print("uint8");
 		public uint8 pub;
 		rand_test(pub,random_tests);
 	}
 	{
 		print("uint16");
 		public uint16 pub;
 		rand_test(pub,random_tests);
 	}
 	{
 		print("uint32");
 		public uint32 pub;
 		rand_test(pub,random_tests);
 	}
 	{
 		print("uint64");
 		public uint64 pub;
 		rand_test(pub,random_tests);
 	}
 	{
 		print("int8");
 		public int8 pub;
 		rand_test(pub,random_tests);
 	}
 	{
 		print("int16");
 		public int16 pub;
 		rand_test(pub,random_tests);
 	}
 	{
 		print("int32");
 		public int32 pub;
 		rand_test(pub,random_tests);
 	}
 	{
 		print("int64");
 		public int64 pub;
 		rand_test(pub,random_tests);
 	}
 	
 	print("Test finished!");
 	print("Succeeded tests: ", succeeded_tests);
 	print("Failed tests: ", all_tests - succeeded_tests);
 }