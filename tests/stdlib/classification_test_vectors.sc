/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module classification_test_vectors;

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

template <type T, dim N>
void test_0a(T[[N]] pub){
 	pd_a3p T[[N]] priv;
 	priv = pub;
 	succeeded_tests = succeeded_tests + 1;
 	all_tests = all_tests +1;
 	print("SUCCESS!");
}

template <domain D : additive3pp, type T, dim N>
void test_0b(D T[[N]] priv){
	public T[[N]] pub;
 	pub = declassify(priv);
 	succeeded_tests = succeeded_tests + 1;
 	all_tests = all_tests +1;
 	print("SUCCESS!");
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

template <type T, dim N>
T[[N]] ran_vec(T[[N]] pub){
	pd_a3p T[[N]] priv = pub;
	priv = randomize(priv);
	pub = declassify(priv);
	return pub;
}

template<domain D : additive3pp ,type T, type T2, dim N>
void xor_test(D T[[N]] priv, T2[[N]] pub){
	T2[[N]] pub2 = declassify(priv);
	if(any(declassify(priv) != pub)){
 		all_tests = all_tests + 1;
 		print("FAILURE! Expected value ",arrayToString(pub)," but got ",arrayToString(declassify(priv)));
 	}
 	else{
 		print("SUCCESS!");
 		all_tests = all_tests + 1;
 		succeeded_tests = succeeded_tests + 1;
 	}
}

public uint32 all_tests;
public uint32 succeeded_tests;
public bool test_result;

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

 	for(uint32 i = 0; i < 3; ++i){
 		FLOAT64_MAX *= FLOAT64_MAX;
 	}
 	FLOAT64_MIN = FLOAT64_MIN - FLOAT64_MAX;
 	FLOAT32_MIN = FLOAT32_MIN - FLOAT32_MAX;

 	print("Classification test: start");

 	print("TEST 0a: PUBLIC -> PRIVATE conversions throws no errors");
 	{
 		print("boolean");
 		public bool[[1]] pub (5) = true;
 		test_0a(pub);
 	}
 	{
 		print("uint8");
 		public uint8[[1]] pub (5) = 1;
 		test_0a(pub);
 	}
 	{
 		print("uint16");
 		public uint16[[1]] pub (5) = 1;
 		test_0a(pub);
 	}
 	{
 		print("uint32");
 		public uint32[[1]] pub (5) = 1;
 		test_0a(pub);
 	}
 	{
 		print("uint64");
 		public uint[[1]] pub (5) = 1;
 		test_0a(pub);
 	}
 	{
 		print("int8");
 		public int8[[1]] pub (5) = 1;
 		test_0a(pub);
 	}
 	{
 		print("int16");
 		public int16[[1]] pub (5) = 1;
 		test_0a(pub);
 	}
 	{
 		print("int32");
 		public int32[[1]] pub (5) = 1;
 		test_0a(pub);
 	}
 	{
 		print("int64/int");
 		public int[[1]] pub (5) = 1;
 		test_0a(pub);
 	}
 	print("TEST 0b: PRIVATE -> PUBLIC conversion throws no errors");
 	{
 		print("boolean");
 		pd_a3p bool[[1]] priv (5) = false;
 		test_0b(priv);
 	}
 	{
 		print("uint8");
 		pd_a3p uint8[[1]] priv (5) = 0;
 		test_0b(priv);
 	}
 	{
 		print("uint16");
 		pd_a3p uint16[[1]] priv (5) = 0;
 		test_0b(priv);
 	}
 	{
 		print("uint32");
 		pd_a3p uint32[[1]] priv (5) = 0;
 		test_0b(priv);
 	}
 	{
 		print("uint64/uint");
 		pd_a3p uint[[1]] priv (5) = 0;
 		test_0b(priv);
 	}
 	{
 		print("int8");
 		pd_a3p int8[[1]] priv (5) = 0;
 		test_0b(priv);
 	}
 	{
 		print("int16");
 		pd_a3p int16[[1]] priv (5) = 0;
 		test_0b(priv);
 	}
 	{
 		print("int32");
 		pd_a3p int32[[1]] priv (5) = 0;
 		test_0b(priv);
 	}
 	{
 		print("int64/int");
 		pd_a3p int[[1]] priv (5) = 0;
 		test_0b(priv);
 	}
 	{
 		print("xor_uint8");
 		pd_a3p xor_uint8[[1]] priv (5) = 0;
 		uint8[[1]] pub (5) = 0;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint16");
 		pd_a3p xor_uint16[[1]] priv (5) = 0;
 		uint16[[1]] pub (5) = 0;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint32");
 		pd_a3p xor_uint32[[1]] priv (5) = 0;
 		uint32[[1]] pub (5) = 0;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint64");
 		pd_a3p xor_uint64[[1]] priv (5) = 0;
 		uint64[[1]] pub (5) = 0;
 		xor_test(priv,pub);
 	}
 	print("TEST 1: PUBLIC -> PRIVATE -> PUBLIC conversion with MAX values");
	{
		print("uint8");
		public uint8[[1]] pub (5) = UINT8_MAX;
		test(pub);
	}
	{
		print("uint16");
		public uint16[[1]] pub (5) = UINT16_MAX;
		test(pub);
	}
	{
		print("uint32");
		public uint32[[1]] pub (5) = UINT32_MAX;
		test(pub);
	}
	{
		print("uint64/uint");
		public uint[[1]] pub (5) = UINT64_MAX;
		test(pub);
	}
	{
		print("int8");
		public int8[[1]] pub (5) = INT8_MAX;
		test(pub);
	}
	{
		print("int16");
		public int16[[1]] pub (5) = INT16_MAX;
		test(pub);
	}
	{
		print("int32");
		public int32[[1]] pub (5) = INT32_MAX;
		test(pub);
	}
	{
		print("int64/int");
		public int64[[1]] pub (5) = INT64_MAX;
		test(pub);
	}
	{
 		print("xor_uint8");
 		pd_a3p xor_uint8[[1]] priv (5) = UINT8_MAX;
 		uint8[[1]] pub (5) = UINT8_MAX;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint16");
 		pd_a3p xor_uint16[[1]] priv (5) = UINT16_MAX;
 		uint16[[1]] pub (5) = UINT16_MAX;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint32");
 		pd_a3p xor_uint32[[1]] priv (5) = UINT32_MAX;
 		uint32[[1]] pub (5) = UINT32_MAX;
 		xor_test(priv,pub);
 	}
 	{
 		print("xor_uint64");
 		pd_a3p xor_uint64[[1]] priv (5) = UINT64_MAX;
 		uint64[[1]] pub (5) = UINT64_MAX;
 		xor_test(priv,pub);
 	}
	print("TEST 2: PUBLIC -> PRIVATE -> PUBLIC conversion with MIN values");
	{
		print("int8");
		public int8[[1]] pub (5) = INT8_MIN;
		test(pub);
	}
	{
		print("int16");
		public int16[[1]] pub (5) = INT16_MIN;
		test(pub);
	}
	{
		print("int32");
		public int32[[1]] pub (5) = INT32_MIN;
		test(pub);
	}
	{
		print("int64/int");
		public int64[[1]] pub (5) = INT64_MIN;
		test(pub);
	}
 	print("TEST 3: PUBLIC -> PRIVATE -> PUBLIC conversion with randomized values over 1-10 element vectors");
	for(uint i = 1; i < 11; ++i){
		{
	 		print("boolean : ", i ," element vector");
	 		public bool[[1]] pub (i);
		 	pub = ran_vec(pub);
		 	test(pub);
	    }	
	 	{
	 		print("uint8: ", i ," element vector");
	 		public uint8[[1]] pub (i);
		 	pub = ran_vec(pub);
		 	test(pub);
	    }
	 	{
	 		print("uint16: ", i ," element vector");
	 		public uint16[[1]] pub (i);
	 		pub = ran_vec(pub);
	 		test(pub);
	 	}
	 	{
	 		print("uint32: ", i ," element vector");
	 		public uint32[[1]] pub (i);
	 		pub = ran_vec(pub);
	 		test(pub);
	 	}
	 	{
	 		print("uint64/uint: ", i ," element vector");
	 		public uint[[1]] pub (i);
	 		pub = ran_vec(pub);
	 		test(pub);
	 	}
	 	{
	 		print("int8: ", i ," element vector");
	 		public int8[[1]] pub (i);
	 		pub = ran_vec(pub);
	 		test(pub);
	 	}
	 	{
	 		print("int16: ", i ," element vector");
	 		public int16[[1]] pub (i);
	 		pub = ran_vec(pub);
	 		test(pub);
	 	}
	 	{
	 		print("int32: ", i ," element vector");
	 		public int32[[1]] pub (i);
	 		pub = ran_vec(pub);
	 		test(pub);
	 	}
	 	{
	 		print("int64/int: ", i ," element vector");
	 		public int64[[1]] pub (i);
	 		pub = ran_vec(pub);
	 		test(pub);
	 	}
 	}
 	
 	print("Test finished!");
 	print("Succeeded tests: ", succeeded_tests);
 	print("Failed tests: ", all_tests - succeeded_tests);
}
