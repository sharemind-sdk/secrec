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


	print("Arithmetic test: start");

	print("TEST 1: Addition with two public values");
	{
		print("uint8");
		uint8 a = 15; uint8 b = 174; uint8 c = a+b;
		if(c == 189){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("uint16");
		uint16 a = 175; uint16 b = 45876; uint16 c = a+b;
		if(c == 46051){Success();}else{Failure("Expected 46051",c);}
	}
	{
		print("uint32");
		uint32 a = 2398; uint32 b = 21798357; uint32 c = a+b;
		if(c == 21800755){Success();}else{Failure("Expected 21800755",c);}
	}
	{
		print("uint");
		uint a = 2578953; uint b = 1872698523698; uint c = a+b;
		if(c == 1872701102651){Success();}else{Failure("Expected 1872701102651",c);}
	}
	{
		print("int8");
		int8 a = -25; int8 b = 50; int8 c = a+b;
		if(c == 25){Success();}else{Failure("Expected 25",c);}
	}
	{
		print("int16");
		int16 a = -2264; int16 b = 22468; int16 c = a+b;
		if(c == 20204){Success();}else{Failure("Expected 20204",c);}
	}
	{
		print("int32");
		int32 a = -12549; int32 b = 21485747; int32 c = a+b;
		if(c == 21473198){Success();}else{Failure("Expected 21800755",c);}
	}
	{
		print("int");
		int a = 2954; int b = 93214654775807; int c = a+b;
		if(c == 93214654778761){Success();}else{Failure("Expected 93214654778761",c);}
	}
	print("TEST 2: Addition with two private values");
	{
		print("uint8");
		pd_a3p uint8 a = 15; pd_a3p uint8 b = 174; pd_a3p uint8 c = a+b;
		if(declassify(c) == 189){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; pd_a3p uint16 b = 45876; pd_a3p uint16 c = a+b;
		if(declassify(c) == 46051){Success();}else{Failure("Expected 46051",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 2398; pd_a3p uint32 b = 21798357; pd_a3p uint32 c = a+b;
		if(declassify(c) == 21800755){Success();}else{Failure("Expected 21800755",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 2578953; pd_a3p uint b = 1872698523698; pd_a3p uint c = a+b;
		if(declassify(c) == 1872701102651){Success();}else{Failure("Expected 1872701102651",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = -25; pd_a3p int8 b = 50; pd_a3p int8 c = a+b;
		if(declassify(c) == 25){Success();}else{Failure("Expected 25",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = -2264; pd_a3p int16 b = 22468; pd_a3p int16 c = a+b;
		if(declassify(c) == 20204){Success();}else{Failure("Expected 20204",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = -12549; pd_a3p int32 b = 21485747; pd_a3p int32 c = a+b;
		if(declassify(c) == 21473198){Success();}else{Failure("Expected 21800755",c);}
	}
	{
		print("int");
		pd_a3p int a = 2954; pd_a3p int b = 93214654775807; pd_a3p int c = a+b;
		if(declassify(c) == 93214654778761){Success();}else{Failure("Expected 93214654778761",c);}
	}
	print("TEST 3: Addition with one private one public value");
	{
		print("uint8");
		pd_a3p uint8 a = 15; uint8 b = 174; pd_a3p uint8 c = a+b;
		if(declassify(c) == 189){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; uint16 b = 45876; pd_a3p uint16 c = a+b;
		if(declassify(c) == 46051){Success();}else{Failure("Expected 46051",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 2398; uint32 b = 21798357; pd_a3p uint32 c = a+b;
		if(declassify(c) == 21800755){Success();}else{Failure("Expected 21800755",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 2578953; uint b = 1872698523698; pd_a3p uint c = a+b;
		if(declassify(c) == 1872701102651){Success();}else{Failure("Expected 1872701102651",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = -25; int8 b = 50; pd_a3p int8 c = a+b;
		if(declassify(c) == 25){Success();}else{Failure("Expected 25",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = -2264; int16 b = 22468; pd_a3p int16 c = a+b;
		if(declassify(c) == 20204){Success();}else{Failure("Expected 20204",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = -12549; int32 b = 21485747; pd_a3p int32 c = a+b;
		if(declassify(c) == 21473198){Success();}else{Failure("Expected 21800755",c);}
	}
	{
		print("int");
		pd_a3p int a = 2954; int b = 93214654775807; pd_a3p int c = a+b;
		if(declassify(c) == 93214654778761){Success();}else{Failure("Expected 93214654778761",c);}
	}
	print("TEST 4: Addition with one private one public value(2)");
	{
		print("uint8");
		pd_a3p uint8 a = 15; uint8 b = 174; pd_a3p uint8 c = b+a;
		if(declassify(c) == 189){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; uint16 b = 45876; pd_a3p uint16 c = b+a;
		if(declassify(c) == 46051){Success();}else{Failure("Expected 46051",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 2398; uint32 b = 21798357; pd_a3p uint32 c = b+a;
		if(declassify(c) == 21800755){Success();}else{Failure("Expected 21800755",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 2578953; uint b = 1872698523698; pd_a3p uint c = b+a;
		if(declassify(c) == 1872701102651){Success();}else{Failure("Expected 1872701102651",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = -25; int8 b = 50; pd_a3p int8 c = b+a;
		if(declassify(c) == 25){Success();}else{Failure("Expected 25",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = -2264; int16 b = 22468; pd_a3p int16 c = b+a;
		if(declassify(c) == 20204){Success();}else{Failure("Expected 20204",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = -12549; int32 b = 21485747; pd_a3p int32 c = b+a;
		if(declassify(c) == 21473198){Success();}else{Failure("Expected 21800755",c);}
	}
	{
		print("int");
		pd_a3p int a = 2954; int b = 93214654775807; pd_a3p int c = b+a;
		if(declassify(c) == 93214654778761){Success();}else{Failure("Expected 93214654778761",c);}
	}
	print("TEST 5: Addition with two public values modulo (type_MAX + 1)");
	{
		print("uint8");
		uint8 a = 1; uint8 b = UINT8_MAX; uint8 c = a+b;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint16");
		uint16 a = 1; uint16 b = UINT16_MAX; uint16 c = a+b;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint32");
		uint32 a = 1; uint32 b = UINT32_MAX; uint32 c = a+b;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint");
		uint a = 1; uint b = UINT64_MAX; uint c = a+b;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int8");
		int8 a = 1; int8 b = INT8_MAX; int8 c = a+b;
		if(c == INT8_MIN){Success();}else{Failure("Expected -128",c);}
	}
	{
		print("int16");
		int16 a = 1; int16 b = INT16_MAX; int16 c = a+b;
		if(c == INT16_MIN){Success();}else{Failure("Expected -32768",c);}
	}
	{
		print("int32");
		int32 a = 1; int32 b = INT32_MAX; int32 c = a+b;
		if(c == INT32_MIN){Success();}else{Failure("Expected -2147483648",c);}
	}
	{
		print("int");
		int a = 1; int b = INT64_MAX; int c = a+b;
		if(c == INT64_MIN){Success();}else{Failure("Expected -9223372036854775808",c);}
	}
	print("TEST 6: Addition with two private values modulo (type_MAX + 1)");
	{
		print("uint8");
		pd_a3p uint8 a = 1; pd_a3p uint8 b = UINT8_MAX; pd_a3p uint8 c = a+b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 1; pd_a3p uint16 b = UINT16_MAX; pd_a3p uint16 c = a+b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 1; pd_a3p uint32 b = UINT32_MAX; pd_a3p uint32 c = a+b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 1; pd_a3p uint b = UINT64_MAX; pd_a3p uint c = a+b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = 1; pd_a3p int8 b = INT8_MAX; pd_a3p int8 c = a+b;
		if(declassify(c) == INT8_MIN){Success();}else{Failure("Expected -128",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = 1; pd_a3p int16 b = INT16_MAX; pd_a3p int16 c = a+b;
		if(declassify(c) == INT16_MIN){Success();}else{Failure("Expected -32768",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = 1; pd_a3p int32 b = INT32_MAX; pd_a3p int32 c = a+b;
		if(declassify(c) == INT32_MIN){Success();}else{Failure("Expected -2147483648",c);}
	}
	{
		print("int");
		pd_a3p int a = 1; pd_a3p int b = INT64_MAX; pd_a3p int c = a+b;
		if(declassify(c) == INT64_MIN){Success();}else{Failure("Expected -9223372036854775808",c);}
	}
	print("TEST 7: Addition with public values modulo (A + T_MAX+1 = A)");
	{
		print("uint8");
		uint8 a = 15; uint8 b = UINT8_MAX; uint8 c = a+b+1;
		if(c == a){Success();}else{Failure("Expected 15",c);}
	}
	{
		print("uint16");
		uint16 a = 175; uint16 b = UINT16_MAX; uint16 c = a+b+1;
		if(c == a){Success();}else{Failure("Expected 175",c);}
	}
	{
		print("uint32");
		uint32 a = 2398; uint32 b = UINT32_MAX; uint32 c = a+b+1;
		if(c == a){Success();}else{Failure("Expected 2398",c);}
	}
	{
		print("uint");
		uint a = 2578953; uint b = UINT64_MAX; uint c = a+b+1;
		if(c == a){Success();}else{Failure("Expected 2578953",c);}
	}
	print("TEST 8: Addition with private values modulo (A + T_MAX+1 = A)");
	{
		print("uint8");
		pd_a3p uint8 a = 15; pd_a3p uint8 b = UINT8_MAX; pd_a3p uint8 c = a+b+1;
		if(declassify(c) == declassify(a)){Success();}else{Failure("Expected 15",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; pd_a3p uint16 b = UINT16_MAX; pd_a3p uint16 c = a+b+1;
		if(declassify(c) == declassify(a)){Success();}else{Failure("Expected 175",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 2398; pd_a3p uint32 b = UINT32_MAX; pd_a3p uint32 c = a+b+1;
		if(declassify(c) == declassify(a)){Success();}else{Failure("Expected 2398",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 2578953; pd_a3p uint b = UINT64_MAX; pd_a3p uint c = a+b+1;
		if(declassify(c) == declassify(a)){Success();}else{Failure("Expected 2578953",c);}
	}












	print("TEST 9: Subtraction with two public values");
	{
		print("uint8");
		uint8 a = 15; uint8 b = 174; uint8 c = b-a;
		if(c == 159){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("uint16");
		uint16 a = 175; uint16 b = 45876; uint16 c = b-a;
		if(c == 45701){Success();}else{Failure("Expected 46051",c);}
	}
	{
		print("uint32");
		uint32 a = 2398; uint32 b = 21798357; uint32 c = b-a;
		if(c == 21795959){Success();}else{Failure("Expected 21795959",c);}
	}
	{
		print("uint");
		uint a = 2578953; uint b = 1872698523698; uint c = b-a;
		if(c == 1872695944745){Success();}else{Failure("Expected 1872695944745",c);}
	}
	{
		print("int8");
		int8 a = 25; int8 b = 50; int8 c = b-a;
		if(c == 25){Success();}else{Failure("Expected 25",c);}
	}
	{
		print("int16");
		int16 a = 42264; int16 b = 22468; int16 c = b-a;
		if(c == -19796){Success();}else{Failure("Expected -19796",c);}
	}
	{
		print("int32");
		int32 a = 12549; int32 b = 21485747; int32 c = b-a;
		if(c == 21473198){Success();}else{Failure("Expected 21473198",c);}
	}
	{
		print("int");
		int a = 2954; int b = 93214654775807; int c = b-a;
		if(c == 93214654772853){Success();}else{Failure("Expected 93214654772853",c);}
	}
	print("TEST 10: Subtraction with two private values");
	{
		print("uint8");
		pd_a3p uint8 a = 15; pd_a3p uint8 b = 174; pd_a3p uint8 c = b-a;
		if(declassify(c) == 159){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; pd_a3p uint16 b = 45876; pd_a3p uint16 c = b-a;
		if(declassify(c) == 45701){Success();}else{Failure("Expected 45701",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 2398; pd_a3p uint32 b = 21798357; pd_a3p uint32 c = b-a;
		if(declassify(c) == 21795959){Success();}else{Failure("Expected 21795959",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 2578953; pd_a3p uint b = 1872698523698; pd_a3p uint c = b-a;
		if(declassify(c) == 1872695944745){Success();}else{Failure("Expected 1872695944745",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = 25; pd_a3p int8 b = 50; pd_a3p int8 c = b-a;
		if(declassify(c) == 25){Success();}else{Failure("Expected 25",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = 42264; pd_a3p int16 b = 22468; pd_a3p int16 c = b-a;
		if(declassify(c) == -19796){Success();}else{Failure("Expected -19796",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = 12549; pd_a3p int32 b = 21485747; pd_a3p int32 c = b-a;
		if(declassify(c) == 21473198){Success();}else{Failure("Expected 21473198",c);}
	}
	{
		print("int");
		pd_a3p int a = 2954; pd_a3p int b = 93214654775807; pd_a3p int c = b-a;
		if(declassify(c) == 93214654772853){Success();}else{Failure("Expected 93214654772853",c);}
	}
	print("TEST 11: Subtraction with one private one public value");
	{
		print("uint8");
		pd_a3p uint8 a = 15; uint8 b = 174; pd_a3p uint8 c = b-a;
		if(declassify(c) == 159){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; uint16 b = 45876; pd_a3p uint16 c = b-a;
		if(declassify(c) == 45701){Success();}else{Failure("Expected 45701",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 2398; uint32 b = 21798357; pd_a3p uint32 c = b-a;
		if(declassify(c) == 21795959){Success();}else{Failure("Expected 21795959",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 2578953; uint b = 1872698523698; pd_a3p uint c = b-a;
		if(declassify(c) == 1872695944745){Success();}else{Failure("Expected 1872695944745",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = 25; int8 b = 50; pd_a3p int8 c = b-a;
		if(declassify(c) == 25){Success();}else{Failure("Expected 25",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = 42264; int16 b = 22468; pd_a3p int16 c = b-a;
		if(declassify(c) == -19796){Success();}else{Failure("Expected -19796",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = 12549; int32 b = 21485747; pd_a3p int32 c = b-a;
		if(declassify(c) == 21473198){Success();}else{Failure("Expected 21473198",c);}
	}
	{
		print("int");
		pd_a3p int a = 2954; int b = 93214654775807; pd_a3p int c = b-a;
		if(declassify(c) == 93214654772853){Success();}else{Failure("Expected 93214654772853",c);}
	}
	print("TEST 12: Subtraction with one private one public value(2)");
	{
		print("uint8");
		pd_a3p uint8 a = 174; uint8 b = 15; pd_a3p uint8 c = a-b;
		if(declassify(c) == 159){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 45876; uint16 b = 175; pd_a3p uint16 c = a-b;
		if(declassify(c) == 45701){Success();}else{Failure("Expected 45701",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 21798357; uint32 b = 2398; pd_a3p uint32 c = a-b;
		if(declassify(c) == 21795959){Success();}else{Failure("Expected 21795959",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 1872698523698; uint b = 2578953; pd_a3p uint c = a-b;
		if(declassify(c) == 1872695944745){Success();}else{Failure("Expected 1872695944745",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = 50; int8 b = 25; pd_a3p int8 c = a-b;
		if(declassify(c) == 25){Success();}else{Failure("Expected 25",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = 22468; int16 b = 42264; pd_a3p int16 c = a-b;
		if(declassify(c) == -19796){Success();}else{Failure("Expected -19796",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = 21485747; int32 b =12549; pd_a3p int32 c = a-b;
		if(declassify(c) == 21473198){Success();}else{Failure("Expected 21473198",c);}
	}
	{
		print("int");
		pd_a3p int a = 93214654775807; int b = 2954; pd_a3p int c = a-b;
		if(declassify(c) == 93214654772853){Success();}else{Failure("Expected 93214654772853",c);}
	}
	print("TEST 13: Subtraction with two public values modulo (type_MIN - 1)");
	{
		print("uint8");
		uint8 a = 1; uint8 b = 0; uint8 c = b-a;
		if(c == UINT8_MAX){Success();}else{Failure("Expected 255",c);}
	}
	{
		print("uint16");
		uint16 a = 1; uint16 b = 0; uint16 c = b-a;
		if(c == UINT16_MAX){Success();}else{Failure("Expected 65535",c);}
	}
	{
		print("uint32");
		uint32 a = 1; uint32 b = 0; uint32 c = b-a;
		if(c == UINT32_MAX){Success();}else{Failure("Expected 4294967295",c);}
	}
	{
		print("uint");
		uint a = 1; uint b = 0; uint c = b-a;
		if(c == UINT64_MAX){Success();}else{Failure("Expected 18446744073709551615",c);}
	}
	{
		print("int8");
		int8 a = 1; int8 b = INT8_MIN; int8 c = b-a;
		if(c == INT8_MAX){Success();}else{Failure("Expected 127",c);}
	}
	{
		print("int16");
		int16 a = 1; int16 b = INT16_MIN; int16 c = b-a;
		if(c == INT16_MAX){Success();}else{Failure("Expected 32767",c);}
	}
	{
		print("int32");
		int32 a = 1; int32 b = INT32_MIN; int32 c = b-a;
		if(c == INT32_MAX){Success();}else{Failure("Expected 2147483647",c);}
	}
	{
		print("int");
		int a = 1; int b = INT64_MIN; int c = b-a;
		if(c == INT64_MAX){Success();}else{Failure("Expected 9223372036854775807",c);}
	}
	print("TEST 14: Subtraction with two private values modulo (type_MIN - 1)");
	{
		print("uint8");
		pd_a3p uint8 a = 1; pd_a3p uint8 b = 0; pd_a3p uint8 c = b-a;
		if(declassify(c) == UINT8_MAX){Success();}else{Failure("Expected 255",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 1; pd_a3p uint16 b = 0; pd_a3p uint16 c = b-a;
		if(declassify(c) == UINT16_MAX){Success();}else{Failure("Expected 65535",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 1; pd_a3p uint32 b = 0; pd_a3p uint32 c = b-a;
		if(declassify(c) == UINT32_MAX){Success();}else{Failure("Expected 4294967295",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 1; pd_a3p uint b = 0; pd_a3p uint c = b-a;
		if(declassify(c) == UINT64_MAX){Success();}else{Failure("Expected 18446744073709551615",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = 1; pd_a3p int8 b = INT8_MIN; pd_a3p int8 c = b-a;
		if(declassify(c) == INT8_MAX){Success();}else{Failure("Expected 127",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = 1; pd_a3p int16 b = INT16_MIN; pd_a3p int16 c = b-a;
		if(declassify(c) == INT16_MAX){Success();}else{Failure("Expected 32767",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = 1; pd_a3p int32 b = INT32_MIN; pd_a3p int32 c = b-a;
		if(declassify(c) == INT32_MAX){Success();}else{Failure("Expected 2147483647",c);}
	}
	{
		print("int");
		pd_a3p int a = 1; pd_a3p int b = INT64_MIN; pd_a3p int c = b-a;
		if(declassify(c) == INT64_MAX){Success();}else{Failure("Expected 9223372036854775807",c);}
	}












	print("TEST 15: Multiplication with two public values");
	{
		print("uint8");
		uint8 a = 15; uint8 b = 12; uint8 c = a*b;
		if(c == 180){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("uint16");
		uint16 a = 175; uint16 b = 139; uint16 c = a*b;
		if(c == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	{
		print("uint32");
		uint32 a = 2398; uint32 b = 4051; uint32 c = a*b;
		if(c == 9714298){Success();}else{Failure("Expected 9714298",c);}
	}
	{
		print("uint");
		uint a = 248924; uint b = 48265; uint c = a*b;
		if(c == 12014316860){Success();}else{Failure("Expected 12014316860",c);}
	}
	{
		print("int8");
		int8 a = 25; int8 b = -4; int8 c = a*b;
		if(c == -100){Success();}else{Failure("Expected -100",c);}
	}
	{
		print("int16");
		int16 a = 175; int16 b = 139; int16 c = a*b;
		if(c == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	{
		print("int32");
		int32 a = -2398; int32 b = 4051; int32 c = a*b;
		if(c == -9714298){Success();}else{Failure("Expected -9714298",c);}
	}
	{
		print("int");
		int a = 248924; int b = 48265; int c = a*b;
		if(c == 12014316860){Success();}else{Failure("Expected 12014316860",c);}
	}
	print("TEST 16: Multiplication with two private values");
	{
		print("uint8");
		pd_a3p uint8 a = 15; pd_a3p uint8 b = 12; pd_a3p uint8 c = a*b;
		if(declassify(c) == 180){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; pd_a3p uint16 b = 139; pd_a3p uint16 c = a*b;
		if(declassify(c) == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 2398; pd_a3p uint32 b = 4051; pd_a3p uint32 c = a*b;
		if(declassify(c) == 9714298){Success();}else{Failure("Expected 9714298",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 248924; pd_a3p uint b = 48265; pd_a3p uint c = a*b;
		if(declassify(c) == 12014316860){Success();}else{Failure("Expected 12014316860",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = 25; pd_a3p int8 b = -4; pd_a3p int8 c = a*b;
		if(declassify(c) == -100){Success();}else{Failure("Expected -100",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = 175; pd_a3p int16 b = 139; pd_a3p int16 c = a*b;
		if(declassify(c) == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = -2398; pd_a3p int32 b = 4051; pd_a3p int32 c = a*b;
		if(declassify(c) == -9714298){Success();}else{Failure("Expected -9714298",c);}
	}
	{
		print("int");
		pd_a3p int a = 248924; pd_a3p int b = 48265; pd_a3p int c = a*b;
		if(declassify(c) == 12014316860){Success();}else{Failure("Expected 12014316860",c);}
	}
	print("TEST 17: Multiplication with one private one public value");
	{
		print("uint8");
		pd_a3p uint8 a = 15; uint8 b = 12; pd_a3p uint8 c = a*b;
		if(declassify(c) == 180){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; uint16 b = 139; pd_a3p uint16 c = a*b;
		if(declassify(c) == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 2398; uint32 b = 4051; pd_a3p uint32 c = a*b;
		if(declassify(c) == 9714298){Success();}else{Failure("Expected 9714298",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 248924; uint b = 48265; pd_a3p uint c = a*b;
		if(declassify(c) == 12014316860){Success();}else{Failure("Expected 12014316860",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = 25; int8 b = -4; pd_a3p int8 c = a*b;
		if(declassify(c) == -100){Success();}else{Failure("Expected -100",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = 175; int16 b = 139; pd_a3p int16 c = a*b;
		if(declassify(c) == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = -2398; int32 b = 4051; pd_a3p int32 c = a*b;
		if(declassify(c) == -9714298){Success();}else{Failure("Expected -9714298",c);}
	}
	{
		print("int");
		pd_a3p int a = 248924; int b = 48265; pd_a3p int c = a*b;
		if(declassify(c) == 12014316860){Success();}else{Failure("Expected 12014316860",c);}
	}
	print("TEST 18: Multiplication with one private one public value(2)");
	{
		print("uint8");
		pd_a3p uint8 a = 15; uint8 b = 12; pd_a3p uint8 c = b*a;
		if(declassify(c) == 180){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; uint16 b = 139; pd_a3p uint16 c = b*a;
		if(declassify(c) == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 2398; uint32 b = 4051; pd_a3p uint32 c = b*a;
		if(declassify(c) == 9714298){Success();}else{Failure("Expected 9714298",c);}
	}
	{
		print("uint");
		pd_a3p uint a = 248924; uint b = 48265; pd_a3p uint c = b*a;
		if(declassify(c) == 12014316860){Success();}else{Failure("Expected 12014316860",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = 25; int8 b = -4; pd_a3p int8 c = b*a;
		if(declassify(c) == -100){Success();}else{Failure("Expected -100",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = 175; int16 b = 139; pd_a3p int16 c = b*a;
		if(declassify(c) == 24325){Success();}else{Failure("Expected 24325",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = -2398; int32 b = 4051; pd_a3p int32 c = b*a;
		if(declassify(c) == -9714298){Success();}else{Failure("Expected -9714298",c);}
	}
	{
		print("int");
		pd_a3p int a = 248924; int b = 48265; pd_a3p int c = b*a;
		if(declassify(c) == 12014316860){Success();}else{Failure("Expected 12014316860",c);}
	}
	print("TEST 19: Multiplication with two public values modulo (type_MAX * 5)");
	{
		print("uint8");
		uint8 a = UINT8_MAX-1; uint8 c = a*5;
		if(c == 246){Success();}else{Failure("Expected 246",c);}
	}
	{
		print("uint16");
		uint16 a = UINT16_MAX-1; uint16 c = a*5;
		if(c == 65526){Success();}else{Failure("Expected 65526",c);}
	}
	{
		print("uint32");
		uint32 a = UINT32_MAX-1; uint32 c = a*5;
		if(c == 4294967286){Success();}else{Failure("Expected 4294967286",c);}
	}
	{
		print("uint");
		uint a = UINT64_MAX-1; uint c = a*5;
		if(c == 18446744073709551606){Success();}else{Failure("Expected 18446744073709551606",c);}
	}
	{
		print("int8");
		int8 a = INT8_MAX-1; int8 c = a*5;
		if(c == 118){Success();}else{Failure("Expected 118",c);}
	}
	{
		print("int16");
		int16 a = INT16_MAX-1; int16 c = a*5;
		if(c == 32758){Success();}else{Failure("Expected 32758",c);}
	}
	{
		print("int32");
		int32 a = INT32_MAX-1; int32 c = a*5;
		if(c == 2147483638){Success();}else{Failure("Expected 2147483638",c);}
	}
	{
		print("int");
		int a = INT64_MAX-1; int c = a*5;
		if(c == 9223372036854775798){Success();}else{Failure("Expected 9223372036854775798",c);}
	}
	print("TEST 20: Multiplication with two private values modulo (type_MAX * 5)");
	{
		print("uint8");
		pd_a3p uint8 a = UINT8_MAX-1; pd_a3p uint8 b = 5; pd_a3p uint8 c = a*b;
		if(declassify(c) == 246){Success();}else{Failure("Expected 246",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = UINT16_MAX-1; pd_a3p uint16 b = 5; pd_a3p uint16 c = a*b;
		if(declassify(c) == 65526){Success();}else{Failure("Expected 65526",c);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = UINT32_MAX-1; pd_a3p uint32 b = 5; pd_a3p uint32 c = a*b;
		if(declassify(c) == 4294967286){Success();}else{Failure("Expected 4294967286",c);}
	}
	{
		print("uint");
		pd_a3p uint a = UINT64_MAX-1; pd_a3p uint b = 5; pd_a3p uint c = a*b;
		if(declassify(c) == 18446744073709551606){Success();}else{Failure("Expected 18446744073709551606",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = INT8_MAX-1; pd_a3p int8 b = 5; pd_a3p int8 c = a*b;
		if(declassify(c) == 118){Success();}else{Failure("Expected 118",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = INT16_MAX-1; pd_a3p int16 b = 5; pd_a3p int16 c = a*b;
		if(declassify(c) == 32758){Success();}else{Failure("Expected 32758",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = INT32_MAX-1; pd_a3p int32 b = 5; pd_a3p int32 c = a*b;
		if(declassify(c) == 2147483638){Success();}else{Failure("Expected 2147483638",c);}
	}
	{
		print("int");
		pd_a3p int a = INT64_MAX-1; pd_a3p int b = 5; pd_a3p int c = a*b;
		if(declassify(c) == 9223372036854775798){Success();}else{Failure("Expected 9223372036854775798",c);}
	}





    /****************************************************************************
    *****************************************************************************
    ***********    Division with private values highly unstable   ***************
    ***********          use only on uint32 values                ***************
	*****************************************************************************
	****************************************************************************/





	print("TEST 21: Division with two public values");
	{
		print("uint8");
		uint8 a = 15; uint8 b = 174; uint8 c = b/a;
		if(c == 11){Success();}else{Failure("Expected 11",c);}
	}
	{
		print("uint16");
		uint16 a = 175; uint16 b = 45876; uint16 c = b/a;
		if(c == 262){Success();}else{Failure("Expected 262",c);}
	}
	{
		print("uint32");
		uint32 a = 2398; uint32 b = 21798357; uint32 c = b/a;
		if(c == 9090){Success();}else{Failure("Expected 9090",c);}
	}
	{
		print("uint");
		uint a = 2578953; uint b = 1872698523698; uint c = b/a;
		if(c == 726146){Success();}else{Failure("Expected 726146",c);}
	}
	{
		print("int8");
		int8 a = 25; int8 b = 50; int8 c = b/a;
		if(c == 2){Success();}else{Failure("Expected 2",c);}
	}
	{
		print("int16");
		int16 a = 42264; int16 b = 22468; int16 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int32");
		int32 a = 12549; int32 b = 21485747; int32 c = b/a;
		if(c == 1712){Success();}else{Failure("Expected 1712",c);}
	}
	{
		print("int");
		int a = 982175129826; int b = 93214654775807; int c = b/a;
		if(c == 94){Success();}else{Failure("Expected 94",c);}
	}
	print("TEST 22: Division with two private values");
	/*{
		print("uint8");
		pd_a3p uint8 a = 15; pd_a3p uint8 b = 174; pd_a3p uint8 c = b/a;
		if(declassify(c) == 11){Success();}else{Failure("Expected 11",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; pd_a3p uint16 b = 45876; pd_a3p uint16 c = b/a;
		if(declassify(c) == 262){Success();}else{Failure("Expected 262",c);}
	}*/
	{
		print("uint32");
		pd_a3p uint32 a = 2398; pd_a3p uint32 b = 21798357; pd_a3p uint32 c = b/a;
		if(declassify(c) == 9090){Success();}else{Failure("Expected 9090",c);}
	}
	/*{
		print("uint");
		pd_a3p uint a = 2578953; pd_a3p uint b = 1872698523698; pd_a3p uint c = b/a;
		if(declassify(c) == 726146){Success();}else{Failure("Expected 726146",c);}
	}*/
	print("TEST 23: Division with one private one public value");
	/*{
		print("uint8");
		pd_a3p uint8 a = 15; uint8 b = 174; pd_a3p uint8 c = b/a;
		if(declassify(c) == 11){Success();}else{Failure("Expected 11",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; uint16 b = 45876; pd_a3p uint16 c = b/a;
		if(declassify(c) == 262){Success();}else{Failure("Expected 262",c);}
	}*/
	{
		print("uint32");
		pd_a3p uint32 a = 2398; uint32 b = 21798357; pd_a3p uint32 c = b/a;
		if(declassify(c) == 9090){Success();}else{Failure("Expected 9090",c);}
	}
	/*{
		print("uint");
		pd_a3p uint a = 2578953; uint b = 1872698523698; pd_a3p uint c = b/a;
		if(declassify(c) == 726146){Success();}else{Failure("Expected 726146",c);}
	}*/
	print("TEST 24: Division with one private one public value(2)");
	/*{
		print("uint8");
		pd_a3p uint8 a = 15; uint8 b = 174; pd_a3p uint8 c = a/b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; uint16 b = 45876; pd_a3p uint16 c = a/b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}*/
	{
		print("uint32");
		pd_a3p uint32 a = 2398; uint32 b = 21798357; pd_a3p uint32 c = a/b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	/*{
		print("uint");
		pd_a3p uint a = 2578953; uint b = 1872698523698; pd_a3p uint c = a/b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}*/
	print("TEST 25: 0 divided with random public values");
	{
		print("uint8");
		uint8 a = 15; uint8 b = 0; uint8 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint16");
		uint16 a = 175; uint16 b = 0; uint16 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint32");
		uint32 a = 2398; uint32 b = 0; uint32 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint");
		uint a = 2578953; uint b = 0; uint c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int8");
		int8 a = 25; int8 b = 0; int8 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int16");
		int16 a = 42264; int16 b = 0; int16 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int32");
		int32 a = 12549; int32 b = 0; int32 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int");
		int a = 982175129826; int b = 0; int c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	print("TEST 26: 0 divided with random private values");
	/*{
		print("uint8");
		pd_a3p uint8 a = 15; pd_a3p uint8 b = 0; pd_a3p uint8 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 175; pd_a3p uint16 b = 0; pd_a3p uint16 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}*/
	{
		print("uint32");
		pd_a3p uint32 a = 2398; pd_a3p uint32 b = 0; pd_a3p uint32 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	/*{
		print("uint");
		pd_a3p uint a = 2578953; pd_a3p uint b = 0; pd_a3p uint c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}*/
	print("TEST 27: A/A = 1 with all public types");
	{
		print("uint8");
		uint8 a = 174; uint8 b = 174; uint8 c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	{
		print("uint16");
		uint16 a = 45876; uint16 b = 45876; uint16 c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	{
		print("uint32");
		uint32 a = 21798357; uint32 b = 21798357; uint32 c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	{
		print("uint");
		uint a = 1872698523698; uint b = 1872698523698; uint c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	{
		print("int8");
		int8 a = 50; int8 b = 50; int8 c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	{
		print("int16");
		int16 a = 22468; int16 b = 22468; int16 c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	{
		print("int32");
		int32 a = 21485747; int32 b = 21485747; int32 c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	{
		print("int");
		int a = 93214654775807; int b = 93214654775807; int c = b/a;
		if(c == 1){Success();}else{Failure("Expected 1",c);}
	}
	print("TEST 28: Division accuracy public");
	{
		print("uint8");
		uint8 a = UINT8_MAX; uint8 b = UINT8_MAX -1 ; uint8 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint16");
		uint16 a = UINT16_MAX; uint16 b = UINT16_MAX -1; uint16 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint32");
		uint32 a = UINT32_MAX; uint32 b = UINT32_MAX -1; uint32 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint");
		uint a = UINT64_MAX; uint b = UINT64_MAX-1; uint c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int8");
		int8 a = INT8_MAX; int8 b = INT8_MAX-1; int8 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int16");
		int16 a = INT16_MAX; int16 b = INT16_MAX-1; int16 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int32");
		int32 a = INT32_MAX; int32 b = INT32_MAX-1; int32 c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int");
		int a = INT64_MAX; int b = INT64_MAX-1; int c = b/a;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	print("TEST 29: Division accuracy private");
	/*{
		print("uint8");
		pd_a3p uint8 a = UINT8_MAX; pd_a3p uint8 b = UINT8_MAX -1 ; pd_a3p uint8 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = UINT16_MAX; pd_a3p uint16 b = UINT16_MAX -1; pd_a3p uint16 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}*/
	{
		print("uint32");
		pd_a3p uint32 a = UINT32_MAX; pd_a3p uint32 b = UINT32_MAX -1; pd_a3p uint32 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	/*{
		print("uint");
		pd_a3p uint a = UINT64_MAX; pd_a3p uint b = UINT64_MAX-1; pd_a3p uint c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int8");
		pd_a3p int8 a = INT8_MAX; pd_a3p int8 b = INT8_MAX-1; pd_a3p int8 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int16");
		pd_a3p int16 a = INT16_MAX; pd_a3p int16 b = INT16_MAX-1; pd_a3p int16 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int32");
		pd_a3p int32 a = INT32_MAX; pd_a3p int32 b = INT32_MAX-1; pd_a3p int32 c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int");
		pd_a3p int a = INT64_MAX; pd_a3p int b = INT64_MAX-1; pd_a3p int c = b/a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}*/










	/****************************************************************************
    *****************************************************************************
    ***********    Modulo for private values implemented but      ***************
    ***********       not working even with uint32 values         ***************
	*****************************************************************************
	****************************************************************************/
	// as a sidenote, using modulo on any private value except uint32 will stop miners 2 and 3
	// and freeze miner 1
	// as for uint32, it will give a SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE


	print("TEST 30: Modulo on public values");
	{
		print("uint8");
		uint8 b = 15; uint8 a = 174; uint8 c = a%b;
		if(c == 9){Success();}else{Failure("Expected 9",c);}
	}
	{
		print("uint16");
		uint16 b = 175; uint16 a = 45876; uint16 c = a%b;
		if(c == 26){Success();}else{Failure("Expected 26",c);}
	}
	{
		print("uint32");
		uint32 b = 2398; uint32 a = 21798357; uint32 c = a%b;
		if(c == 537){Success();}else{Failure("Expected 537",c);}
	}
	{
		print("uint");
		uint b = 2578953; uint a = 1872698523698; uint c = a%b;
		if(c == 2118560){Success();}else{Failure("Expected 2118560",c);}
	}
	{
		print("int8");
		int8 b = -25; int8 a = 50; int8 c = a%b;
		if(c == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int16");
		int16 b = -2264; int16 a = 22468; int16 c = a%b;
		if(c == 2092){Success();}else{Failure("Expected 2092",c);}
	}
	{
		print("int32");
		int32 b = -12549; int32 a = 21485747; int32 c = a%b;
		if(c == 1859){Success();}else{Failure("Expected 1859",c);}
	}
	{
		print("int");
		int b = 2954; int a = 93214654775807; int c = a%b;
		if(c == 257){Success();}else{Failure("Expected 257",c);}
	}
	print("TEST 31: Modulo private values");
	/*{
		print("uint8");
		pd_a3p uint8 b = 15; pd_a3p uint8 a = 174; pd_a3p uint8 c = a%b;
		if(declassify(c) == 9){Success();}else{Failure("Expected 9",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 b = 175; pd_a3p uint16 a = 45876; pd_a3p uint16 c = a%b;
		if(declassify(c) == 26){Success();}else{Failure("Expected 26",c);}
	}
	{
		// unlike other data types, uint32 throws SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE
		// other data types stop miners 2 and 3;
		print("uint32");
		pd_a3p uint32 b = 2398; pd_a3p uint32 a = 21798357; pd_a3p uint32 c = a%b;
		if(declassify(c) == 537){Success();}else{Failure("Expected 537",c);}
	}
	{
		print("uint");
		pd_a3p uint b = 2578953; pd_a3p uint a = 1872698523698; pd_a3p uint c = a%b;
		if(declassify(c) == 2118560){Success();}else{Failure("Expected 2118560",c);}
	}
	{
		print("int8");
		pd_a3p int8 b = -25; pd_a3p int8 a = 50; pd_a3p int8 c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int16");
		pd_a3p int16 b = -2264; pd_a3p int16 a = 22468; pd_a3p int16 c = a%b;
		if(declassify(c) == 2092){Success();}else{Failure("Expected 2092",c);}
	}
	{
		print("int32");
		pd_a3p int32 b = -12549; pd_a3p int32 a = 21485747; pd_a3p int32 c = a%b;
		if(declassify(c) == 1859){Success();}else{Failure("Expected 1859",c);}
	}
	{
		print("int");
		pd_a3p int b = 2954; pd_a3p int a = 93214654775807; pd_a3p int c = a%b;
		if(declassify(c) == 257){Success();}else{Failure("Expected 257",c);}
	}*/
	print("TEST 32: Modulo with private and public values");
	/*{
		print("uint8");
		pd_a3p uint8 b = 15; uint8 a = 174; pd_a3p uint8 c = a%b;
		if(declassify(c) == 9){Success();}else{Failure("Expected 9",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 b = 175; uint16 a = 45876; pd_a3p uint16 c = a%b;
		if(declassify(c) == 26){Success();}else{Failure("Expected 26",c);}
	}
	{
		// unlike other data types, uint32 throws SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE
		// other data types stop miners 2 and 3;
		print("uint32");
		pd_a3p uint32 b = 2398; uint32 a = 21798357; pd_a3p uint32 c = a%b;
		if(declassify(c) == 537){Success();}else{Failure("Expected 537",c);}
	}
	{
		print("uint");
		pd_a3p uint b = 2578953; uint a = 1872698523698; pd_a3p uint c = a%b;
		if(declassify(c) == 2118560){Success();}else{Failure("Expected 2118560",c);}
	}
	{
		print("int8");
		pd_a3p int8 b = -25; int8 a = 50; pd_a3p int8 c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int16");
		pd_a3p int16 b = -2264; int16 a = 22468; pd_a3p int16 c = a%b;
		if(declassify(c) == 2092){Success();}else{Failure("Expected 2092",c);}
	}
	{
		print("int32");
		pd_a3p int32 b = -12549; int32 a = 21485747; pd_a3p int32 c = a%b;
		if(declassify(c) == 1859){Success();}else{Failure("Expected 1859",c);}
	}
	{
		print("int");
		pd_a3p int b = 2954; int a = 93214654775807; pd_a3p int c = a%b;
		if(declassify(c) == 257){Success();}else{Failure("Expected 257",c);}
	}*/
	print("TEST 33: Modulo with private and public values(2)");
	/*{
		print("uint8");
		pd_a3p uint8 b = 174; uint8 a = 15; pd_a3p uint8 c = b%a;
		if(declassify(c) == 9){Success();}else{Failure("Expected 9",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 b = 45876; uint16 a = 175 ; pd_a3p uint16 c = b%a;
		if(declassify(c) == 26){Success();}else{Failure("Expected 26",c);}
	}
	{
		// unlike other data types, uint32 throws SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE
		// other data types stop miners 2 and 3;
		print("uint32");
		pd_a3p uint32 b = 21798357; uint32 a = 2398 ; pd_a3p uint32 c = b%a;
		if(declassify(c) == 537){Success();}else{Failure("Expected 537",c);}
	}
	{
		print("uint");
		pd_a3p uint b = 1872698523698; uint a = 2578953; pd_a3p uint c = b%a;
		if(declassify(c) == 2118560){Success();}else{Failure("Expected 2118560",c);}
	}
	{
		print("int8");
		pd_a3p int8 b = 50; int8 a = -25; pd_a3p int8 c = b%a;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int16");
		pd_a3p int16 b = 22468; int16 a = -2264; pd_a3p int16 c = b%a;
		if(declassify(c) == 2092){Success();}else{Failure("Expected 2092",c);}
	}
	{
		print("int32");
		pd_a3p int32 b = 21485747; int32 a = -12549; pd_a3p int32 c = b%a;
		if(declassify(c) == 1859){Success();}else{Failure("Expected 1859",c);}
	}
	{
		print("int");
		pd_a3p int b = 93214654775807; int a = 2954; pd_a3p int c = b%a;
		if(declassify(c) == 257){Success();}else{Failure("Expected 257",c);}
	}*/
	print("TEST 34: Modulo with important private values");
	/*{
		print("uint8");
		pd_a3p uint8 b = 1; pd_a3p uint8 a = 0; pd_a3p uint8 c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = 1;  a = 1;  c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = UINT8_MAX;  a = UINT8_MAX;  c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint16");
		pd_a3p uint16 b = 1; pd_a3p uint16 a = 0; pd_a3p uint16 c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = 1;  a = 1; c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = UINT16_MAX;a = UINT16_MAX;c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		// unlike other data types, uint32 throws SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE
		// other data types stop miners 2 and 3;
		print("uint32");
		pd_a3p uint32 b = 1; pd_a3p uint32 a = 0; pd_a3p uint32 c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = 1; a = 1; c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = UINT32_MAX; a = UINT32_MAX; c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("uint");
		pd_a3p uint b = 1; pd_a3p uint a = 0; pd_a3p uint c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = 1; a = 1;c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = UINT64_MAX; a = UINT64_MAX;  c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int8");
		pd_a3p int8 b = 1; pd_a3p int8 a = 0; pd_a3p int8 c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = 1;  a = 1;  c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = INT8_MAX;  a = INT8_MAX;  c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int16");
		pd_a3p int16 b = 1; pd_a3p int16 a = 0; pd_a3p int16 c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = 1;a = 1;c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = INT16_MAX;a = INT16_MAX;c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int32");
		pd_a3p int32 b = 1; pd_a3p int32 a = 0; pd_a3p int32 c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = 1;a = 1;c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = INT32_MAX;a = INT32_MAX;c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("int");
		pd_a3p int b = 1; pd_a3p int a = 0; pd_a3p int c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = 1;a = 1;c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}

		b = INT64_MAX;a = INT64_MAX;c = a%b;
		if(declassify(c) == 0){Success();}else{Failure("Expected 0",c);}
	}*/













	print("TEST 35: Operation priorities : Multiplication over addition");
	{
		print("uint8");
		uint8 a = 5; uint8 b = 20; uint8 c = 45;
		uint8 d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",d);}
	}
	{
		print("uint16");
		uint16 a = 5; uint16 b = 20; uint16 c = 45;
		uint16 d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",d);}
	}
	{
		print("uint32");
		uint32 a = 5; uint32 b = 20; uint32 c = 45;
		uint32 d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",d);}
	}
	{
		print("uint");
		uint a = 5; uint b = 20; uint c = 45;
		uint d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",d);}
	}
	{
		print("int8");
		int8 a = 5; int8 b = 20; int8 c = 45;
		int8 d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",d);}
	}
	{
		print("int16");
		int16 a = 5; int16 b = 20; int16 c = 45;
		int16 d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",d);}
	}
	{
		print("int32");
		uint32 a = 5; uint32 b = 20; uint32 c = 45;
		uint32 d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",d);}
	}
	{
		print("int");
		int a = 5; int b = 20; int c = 45;
		int d = c + b * a;
		if(d == 145){Success();}else{Failure("Expected 145",c);}
	}
	print("TEST 36: Operation priorities : Parentheses over multiplication");
	{
		print("uint8");
		uint8 a = 5; uint8 b = 5; uint8 c = 20;
		uint8 d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	{
		print("uint16");
		uint16 a = 5; uint16 b = 5; uint16 c = 20;
		uint16 d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	{
		print("uint32");
		uint32 a = 5; uint32 b = 5; uint32 c = 20;
		uint32 d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	{
		print("uint");
		uint a = 5; uint b = 5; uint c = 20;
		uint d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	{
		print("int8");
		int8 a = 5; int8 b = 5; int8 c = 20;
		int8 d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	{
		print("int16");
		int16 a = 5; int16 b = 5; int16 c = 20;
		int16 d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	{
		print("int32");
		uint32 a = 5; uint32 b = 5; uint32 c = 20;
		uint32 d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	{
		print("int");
		int a = 5; int b = 5; int c = 20;
		int d = (c + b) * a;
		if(d == 125){Success();}else{Failure("Expected 125",d);}
	}
	print("TEST 37: Operation priorities : Division over addition and subtraction");
	{
		print("uint8");
		uint8 a = 5; uint8 b = 5; uint8 c = 20; uint8 d = 5;
		uint8 e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}
	{
		print("uint16");
		uint16 a = 5; uint16 b = 5; uint16 c = 20; uint16 d = 5;
		uint16 e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}
	{
		print("uint32");
		uint32 a = 5; uint32 b = 5; uint32 c = 20; uint32 d = 5;
		uint32 e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}
	{
		print("uint");
		uint a = 5; uint b = 5; uint c = 20; uint d = 5;
		uint e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}
	{
		print("int8");
		int8 a = 5; int8 b = 5; int8 c = 20; int8 d = 5;
		int8 e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}
	{
		print("int16");
		int16 a = 5; int16 b = 5; int16 c = 20; int16 d = 5;
		int16 e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}
	{
		print("int32");
		int32 a = 5; int32 b = 5; int32 c = 20; int32 d = 5;
		int32 e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}
	{
		print("int");
		int a = 5; int b = 5; int c = 20; int d = 5;
		int e = c - a + b / d;
		if(e == 16){Success();}else{Failure("Expected 16",e);}
	}

	print("TEST 38: public boolean negotiation (!)");
	{
		print("bool");
		bool a = true;
		if(a != false){Success();}
		else{Failure("Boolean negotiation failed",false);}
		a = false;
		if(a != true){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("uint8");
		uint8 a = 25;
		uint8 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("uint16");
		uint16 a = 25;
		uint16 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("uint32");
		uint32 a = 25;
		uint32 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("uint64");
		uint64 a = 25;
		uint64 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("int8");
		int8 a = 25;
		int8 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("int16");
		int16 a = 25;
		int16 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("int32");
		int32 a = 25;
		int32 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("int64/int");
		int64 a = 25;
		int64 b = 26;
		if(a != b){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	print("TEST 38: private boolean negotiation (!)");
	{
		print("bool");
		pd_a3p bool a = true;
		pd_a3p bool b = false;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}

		if(declassify(b != a)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("uint8");
		pd_a3p uint8 a = 25;
		pd_a3p uint8 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("uint16");
		pd_a3p uint16 a = 25;
		pd_a3p uint16 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("uint32");
		pd_a3p uint32 a = 25;
		pd_a3p uint32 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("uint64");
		pd_a3p uint64 a = 25;
		pd_a3p uint64 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("int8");
		pd_a3p int8 a = 25;
		pd_a3p int8 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("int16");
		pd_a3p int16 a = 25;
		pd_a3p int16 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("int32");
		pd_a3p int32 a = 25;
		pd_a3p int32 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("int64/int");
		pd_a3p int64 a = 25;
		pd_a3p int64 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8 a = 25;
		pd_a3p xor_uint8 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 a = 25;
		pd_a3p xor_uint16 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 a = 25;
		pd_a3p xor_uint32 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 a = 25;
		pd_a3p xor_uint64 b = 26;
		if(declassify(a != b)){Success();}
		else{Failure("Boolean negotiation failed",false);}
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}