/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module Arithmetic_test_vector;

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
import test_utility;

domain pd_a3p additive3pp;

public uint all_tests;
public uint succeeded_tests;

void Success(){
	succeeded_tests = succeeded_tests + 1;
	all_tests = all_tests +1;
	print("SUCCESS!");
}
void Failure(string s){
	print("FAILURE! ",s);
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

	print("TEST 2: Addition with two private vectors");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; pd_a3p uint8[[1]] b (15)= 174; pd_a3p uint8[[1]] c = a+b;
		if(all(declassify(c) == 189)){Success();}else{Failure("Expected 189");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; pd_a3p uint16[[1]] b (15)= 45876; pd_a3p uint16[[1]] c = a+b;
		if(all(declassify(c) == 46051)){Success();}else{Failure("Expected 46051");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; pd_a3p uint32[[1]] b (15)= 21798357; pd_a3p uint32[[1]] c = a+b;
		if(all(declassify(c) == 21800755)){Success();}else{Failure("Expected 21800755");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; pd_a3p uint[[1]] b (15)= 1872698523698; pd_a3p uint[[1]] c = a+b;
		if(all(declassify(c) == 1872701102651)){Success();}else{Failure("Expected 1872701102651");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= -25; pd_a3p int8[[1]] b (15)= 50; pd_a3p int8[[1]] c = a+b;
		if(all(declassify(c) == 25)){Success();}else{Failure("Expected 25");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= -2264; pd_a3p int16[[1]] b (15)= 22468; pd_a3p int16[[1]] c = a+b;
		if(all(declassify(c) == 20204)){Success();}else{Failure("Expected 20204");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= -12549; pd_a3p int32[[1]] b (15)= 21485747; pd_a3p int32[[1]] c = a+b;
		if(all(declassify(c) == 21473198)){Success();}else{Failure("Expected 21800755");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 2954; pd_a3p int[[1]] b (15)= 93214654775807; pd_a3p int[[1]] c = a+b;
		if(all(declassify(c) == 93214654778761)){Success();}else{Failure("Expected 93214654778761");}
	}
	print("TEST 3: Addition with one private one public vector");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 174; pd_a3p uint8[[1]] c = a+b;
		if(all(declassify(c) == 189)){Success();}else{Failure("Expected 189");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 45876; pd_a3p uint16[[1]] c = a+b;
		if(all(declassify(c) == 46051)){Success();}else{Failure("Expected 46051");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 21798357; pd_a3p uint32[[1]] c = a+b;
		if(all(declassify(c) == 21800755)){Success();}else{Failure("Expected 21800755");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 1872698523698; pd_a3p uint[[1]] c = a+b;
		if(all(declassify(c) == 1872701102651)){Success();}else{Failure("Expected 1872701102651");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= -25; int8[[1]] b (15)= 50; pd_a3p int8[[1]] c = a+b;
		if(all(declassify(c) == 25)){Success();}else{Failure("Expected 25");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= -2264; int16[[1]] b (15)= 22468; pd_a3p int16[[1]] c = a+b;
		if(all(declassify(c) == 20204)){Success();}else{Failure("Expected 20204");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= -12549; int32[[1]] b (15)= 21485747; pd_a3p int32[[1]] c = a+b;
		if(all(declassify(c) == 21473198)){Success();}else{Failure("Expected 21800755");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 2954; int[[1]] b (15)= 93214654775807; pd_a3p int[[1]] c = a+b;
		if(all(declassify(c) == 93214654778761)){Success();}else{Failure("Expected 93214654778761");}
	}
	print("TEST 4: Addition with one private one public vector(2)");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 174; pd_a3p uint8[[1]] c = b+a;
		if(all(declassify(c) == 189)){Success();}else{Failure("Expected 189");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 45876; pd_a3p uint16[[1]] c = b+a;
		if(all(declassify(c) == 46051)){Success();}else{Failure("Expected 46051");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 21798357; pd_a3p uint32[[1]] c = b+a;
		if(all(declassify(c) == 21800755)){Success();}else{Failure("Expected 21800755");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 1872698523698; pd_a3p uint[[1]] c = b+a;
		if(all(declassify(c) == 1872701102651)){Success();}else{Failure("Expected 1872701102651");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= -25; int8[[1]] b (15)= 50; pd_a3p int8[[1]] c = b+a;
		if(all(declassify(c) == 25)){Success();}else{Failure("Expected 25");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= -2264; int16[[1]] b (15)= 22468; pd_a3p int16[[1]] c = b+a;
		if(all(declassify(c) == 20204)){Success();}else{Failure("Expected 20204");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= -12549; int32[[1]] b (15)= 21485747; pd_a3p int32[[1]] c = b+a;
		if(all(declassify(c) == 21473198)){Success();}else{Failure("Expected 21800755");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 2954; int[[1]] b (15)= 93214654775807; pd_a3p int[[1]] c = b+a;
		if(all(declassify(c) == 93214654778761)){Success();}else{Failure("Expected 93214654778761");}
	}
	print("TEST 6: Addition with two private vectors modulo (type_MAX + 1)");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 1; pd_a3p uint8[[1]] b (15)= UINT8_MAX; pd_a3p uint8[[1]] c = a+b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 1; pd_a3p uint16[[1]] b (15)= UINT16_MAX; pd_a3p uint16[[1]] c = a+b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 1; pd_a3p uint32[[1]] b (15)= UINT32_MAX; pd_a3p uint32[[1]] c = a+b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 1; pd_a3p uint[[1]] b (15)= UINT64_MAX; pd_a3p uint[[1]] c = a+b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= 1; pd_a3p int8[[1]] b (15)= INT8_MAX; pd_a3p int8[[1]] c = a+b;
		if(all(declassify(c) == INT8_MIN)){Success();}else{Failure("Expected -128");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= 1; pd_a3p int16[[1]] b (15)= INT16_MAX; pd_a3p int16[[1]] c = a+b;
		if(all(declassify(c) == INT16_MIN)){Success();}else{Failure("Expected -32768");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= 1; pd_a3p int32[[1]] b (15)= INT32_MAX; pd_a3p int32[[1]] c = a+b;
		if(all(declassify(c) == INT32_MIN)){Success();}else{Failure("Expected -2147483648");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 1; pd_a3p int[[1]] b (15)= INT64_MAX; pd_a3p int[[1]] c = a+b;
		if(all(declassify(c) == INT64_MIN)){Success();}else{Failure("Expected -9223372036854775808");}
	}
	print("TEST 8: Addition with private vectors modulo (A + T_MAX+1 = A)");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; pd_a3p uint8[[1]] b (15)= UINT8_MAX; pd_a3p uint8[[1]] c = a+b+1;
		if(all(declassify(c) == declassify(a))){Success();}else{Failure("Expected 15");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; pd_a3p uint16[[1]] b (15)= UINT16_MAX; pd_a3p uint16[[1]] c = a+b+1;
		if(all(declassify(c) == declassify(a))){Success();}else{Failure("Expected 175");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; pd_a3p uint32[[1]] b (15)= UINT32_MAX; pd_a3p uint32[[1]] c = a+b+1;
		if(all(declassify(c) == declassify(a))){Success();}else{Failure("Expected 2398");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; pd_a3p uint[[1]] b (15)= UINT64_MAX; pd_a3p uint[[1]] c = a+b+1;
		if(all(declassify(c) == declassify(a))){Success();}else{Failure("Expected 2578953");}
	}
	print("TEST 10: Subtraction with two private vectors");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; pd_a3p uint8[[1]] b (15)= 174; pd_a3p uint8[[1]] c = b-a;
		if(all(declassify(c) == 159)){Success();}else{Failure("Expected 159");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; pd_a3p uint16[[1]] b (15)= 45876; pd_a3p uint16[[1]] c = b-a;
		if(all(declassify(c) == 45701)){Success();}else{Failure("Expected 46051");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; pd_a3p uint32[[1]] b (15) = 21798357; pd_a3p uint32[[1]] c = b-a;
		if(all(declassify(c) == 21795959)){Success();}else{Failure("Expected 21795959");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; pd_a3p uint[[1]] b (15)= 1872698523698; pd_a3p uint[[1]] c = b-a;
		if(all(declassify(c) == 1872695944745)){Success();}else{Failure("Expected 1872695944745");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= 25; pd_a3p int8[[1]] b (15)= 50; pd_a3p int8[[1]] c = b-a;
		if(all(declassify(c) == 25)){Success();}else{Failure("Expected 25");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= 42264; pd_a3p int16[[1]] b (15)= 22468; pd_a3p int16[[1]] c = b-a;
		if(all(declassify(c) == -19796)){Success();}else{Failure("Expected -19796");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= 12549; pd_a3p int32[[1]] b (15)= 21485747; pd_a3p int32[[1]] c = b-a;
		if(all(declassify(c) == 21473198)){Success();}else{Failure("Expected 21473198");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 2954; pd_a3p int[[1]] b (15) = 93214654775807; pd_a3p int[[1]] c = b-a;
		if(all(declassify(c) == 93214654772853)){Success();}else{Failure("Expected 93214654772853");}
	}
	print("TEST 11: Subtraction with one private one public vector");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 174; pd_a3p uint8[[1]] c = b-a;
		if(all(declassify(c) == 159)){Success();}else{Failure("Expected 159");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 45876; pd_a3p uint16[[1]] c = b-a;
		if(all(declassify(c) == 45701)){Success();}else{Failure("Expected 46051");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; uint32[[1]] b (15) = 21798357; pd_a3p uint32[[1]] c = b-a;
		if(all(declassify(c) == 21795959)){Success();}else{Failure("Expected 21795959");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 1872698523698; pd_a3p uint[[1]] c = b-a;
		if(all(declassify(c) == 1872695944745)){Success();}else{Failure("Expected 1872695944745");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= 25; int8[[1]] b (15)= 50; pd_a3p int8[[1]] c = b-a;
		if(all(declassify(c) == 25)){Success();}else{Failure("Expected 25");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= 42264; int16[[1]] b (15)= 22468; pd_a3p int16[[1]] c = b-a;
		if(all(declassify(c) == -19796)){Success();}else{Failure("Expected -19796");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= 12549; int32[[1]] b (15)= 21485747; pd_a3p int32[[1]] c = b-a;
		if(all(declassify(c) == 21473198)){Success();}else{Failure("Expected 21473198");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 2954; int[[1]] b (15) = 93214654775807; pd_a3p int[[1]] c = b-a;
		if(all(declassify(c) == 93214654772853)){Success();}else{Failure("Expected 93214654772853");}
	}
	print("TEST 12: Subtraction with one private one public value(2)");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 174; uint8[[1]] b (15)= 15; pd_a3p uint8[[1]] c = a-b;
		if(all(declassify(c) == 159)){Success();}else{Failure("Expected 159");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 45876; uint16[[1]] b (15)= 175; pd_a3p uint16[[1]] c = a-b;
		if(all(declassify(c) == 45701)){Success();}else{Failure("Expected 45701");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 21798357; uint32[[1]] b (15)= 2398; pd_a3p uint32[[1]]c = a-b;
		if(all(declassify(c) == 21795959)){Success();}else{Failure("Expected 21795959");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 1872698523698; uint[[1]] b (15)= 2578953; pd_a3p uint[[1]]c = a-b;
		if(all(declassify(c) == 1872695944745)){Success();}else{Failure("Expected 1872695944745");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= 50; int8[[1]] b (15)= 25; pd_a3p int8[[1]] c = a-b;
		if(all(declassify(c) == 25)){Success();}else{Failure("Expected 25");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= 22468; int16[[1]] b (15)= 42264; pd_a3p int16[[1]] c = a-b;
		if(all(declassify(c) == -19796)){Success();}else{Failure("Expected -19796");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= 21485747; int32[[1]] b (15)=12549; pd_a3p int32[[1]] c = a-b;
		if(all(declassify(c) == 21473198)){Success();}else{Failure("Expected 21473198");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 93214654775807; int[[1]] b (15)= 2954; pd_a3p int[[1]] c = a-b;
		if(all(declassify(c) == 93214654772853)){Success();}else{Failure("Expected 93214654772853");}
	}
	print("TEST 14: Subtraction with two private vectors modulo (type_MIN - 1)");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 1; pd_a3p uint8[[1]] b (15)= 0; pd_a3p uint8[[1]] c = b-a;
		if(all(declassify(c) == UINT8_MAX)){Success();}else{Failure("Expected 255");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 1; pd_a3p uint16[[1]] b (15)= 0; pd_a3p uint16[[1]] c = b-a;
		if(all(declassify(c) == UINT16_MAX)){Success();}else{Failure("Expected 65535");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 1; pd_a3p uint32[[1]] b (15)= 0; pd_a3p uint32[[1]] c = b-a;
		if(all(declassify(c) == UINT32_MAX)){Success();}else{Failure("Expected 4294967295");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 1; pd_a3p uint[[1]] b (15)= 0; pd_a3p uint[[1]] c = b-a;
		if(all(declassify(c) == UINT64_MAX)){Success();}else{Failure("Expected 18446744073709551615");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= 1; pd_a3p int8[[1]] b (15)= INT8_MIN; pd_a3p int8[[1]] c = b-a;
		if(all(declassify(c) == INT8_MAX)){Success();}else{Failure("Expected 127");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= 1; pd_a3p int16[[1]] b (15)= INT16_MIN; pd_a3p int16[[1]] c = b-a;
		if(all(declassify(c) == INT16_MAX)){Success();}else{Failure("Expected 32767");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= 1; pd_a3p int32[[1]] b (15)= INT32_MIN; pd_a3p int32[[1]] c = b-a;
		if(all(declassify(c) == INT32_MAX)){Success();}else{Failure("Expected 2147483647");}
	}
	{
		print("int");
		pd_a3p int[[1]] a = 1; pd_a3p int[[1]] b = INT64_MIN; pd_a3p int[[1]] c = b-a;
		if(all(declassify(c) == INT64_MAX)){Success();}else{Failure("Expected 9223372036854775807");}
	}
	print("TEST 16: Multiplication with two private vectors");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; pd_a3p uint8[[1]] b (15)= 12; pd_a3p uint8[[1]] c = a*b;
		if(all(declassify(c) == 180)){Success();}else{Failure("Expected 180");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; pd_a3p uint16[[1]] b (15)= 139; pd_a3p uint16[[1]] c = a*b;
		if(all(declassify(c) == 24325)){Success();}else{Failure("Expected 24325");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; pd_a3p uint32[[1]] b (15)= 4051; pd_a3p uint32[[1]] c = a*b;
		if(all(declassify(c) == 9714298)){Success();}else{Failure("Expected 9714298");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 248924; pd_a3p uint[[1]] b (15)= 48265; pd_a3p uint[[1]] c = a*b;
		if(all(declassify(c) == 12014316860)){Success();}else{Failure("Expected 12014316860");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= 25; pd_a3p int8[[1]] b (15)= -4; pd_a3p int8[[1]] c = a*b;
		if(all(declassify(c) == -100)){Success();}else{Failure("Expected -100");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= 175; pd_a3p int16[[1]] b (15)= 139; pd_a3p int16[[1]] c = a*b;
		if(all(declassify(c) == 24325)){Success();}else{Failure("Expected 24325");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= -2398; pd_a3p int32[[1]] b (15)= 4051; pd_a3p int32[[1]] c = a*b;
		if(all(declassify(c) == -9714298)){Success();}else{Failure("Expected -9714298");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 248924; pd_a3p int[[1]] b (15)= 48265; pd_a3p int[[1]] c = a*b;
		if(all(declassify(c) == 12014316860)){Success();}else{Failure("Expected 12014316860");}
	}
	print("TEST 17: Multiplication with one private one public vector");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 12; pd_a3p uint8[[1]] c = a*b;
		if(all(declassify(c) == 180)){Success();}else{Failure("Expected 180");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 139; pd_a3p uint16[[1]] c = a*b;
		if(all(declassify(c) == 24325)){Success();}else{Failure("Expected 24325");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 4051; pd_a3p uint32[[1]] c = a*b;
		if(all(declassify(c) == 9714298)){Success();}else{Failure("Expected 9714298");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 248924; uint[[1]] b (15)= 48265; pd_a3p uint[[1]] c = a*b;
		if(all(declassify(c) == 12014316860)){Success();}else{Failure("Expected 12014316860");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= 25; int8[[1]] b (15)= -4; pd_a3p int8[[1]] c = a*b;
		if(all(declassify(c) == -100)){Success();}else{Failure("Expected -100");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= 175; int16[[1]] b (15)= 139; pd_a3p int16[[1]] c = a*b;
		if(all(declassify(c) == 24325)){Success();}else{Failure("Expected 24325");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= -2398; int32[[1]] b (15)= 4051; pd_a3p int32[[1]] c = a*b;
		if(all(declassify(c) == -9714298)){Success();}else{Failure("Expected -9714298");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 248924; int[[1]] b (15)= 48265; pd_a3p int[[1]] c = a*b;
		if(all(declassify(c) == 12014316860)){Success();}else{Failure("Expected 12014316860");}
	}
	print("TEST 18: Multiplication with one private one public value(2)");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 12; pd_a3p uint8[[1]] c = b*a;
		if(all(declassify(c) == 180)){Success();}else{Failure("Expected 180");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 139; pd_a3p uint16[[1]] c = b*a;
		if(all(declassify(c) == 24325)){Success();}else{Failure("Expected 24325");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 4051; pd_a3p uint32[[1]] c = b*a;
		if(all(declassify(c) == 9714298)){Success();}else{Failure("Expected 9714298");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 248924; uint[[1]] b (15)= 48265; pd_a3p uint[[1]] c = b*a;
		if(all(declassify(c) == 12014316860)){Success();}else{Failure("Expected 12014316860");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= 25; int8[[1]] b (15)= -4; pd_a3p int8[[1]] c = b*a;
		if(all(declassify(c) == -100)){Success();}else{Failure("Expected -100");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= 175; int16[[1]] b (15)= 139; pd_a3p int16[[1]] c = b*a;
		if(all(declassify(c) == 24325)){Success();}else{Failure("Expected 24325");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= -2398; int32[[1]] b (15)= 4051; pd_a3p int32[[1]] c = b*a;
		if(all(declassify(c) == -9714298)){Success();}else{Failure("Expected -9714298");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= 248924; int[[1]] b (15)= 48265; pd_a3p int[[1]] c = b*a;
		if(all(declassify(c) == 12014316860)){Success();}else{Failure("Expected 12014316860");}
	}
	print("TEST 20: Multiplication with two private values modulo (type_MAX * 5)");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= UINT8_MAX-1; pd_a3p uint8[[1]] b (15)= 5; pd_a3p uint8[[1]] c = a*b;
		if(all(declassify(c) == 246)){Success();}else{Failure("Expected 246");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= UINT16_MAX-1; pd_a3p uint16[[1]] b (15)= 5; pd_a3p uint16[[1]] c = a*b;
		if(all(declassify(c) == 65526)){Success();}else{Failure("Expected 65526");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= UINT32_MAX-1; pd_a3p uint32[[1]] b (15)= 5; pd_a3p uint32[[1]] c = a*b;
		if(all(declassify(c) == 4294967286)){Success();}else{Failure("Expected 4294967286");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= UINT64_MAX-1; pd_a3p uint[[1]] b (15)= 5; pd_a3p uint[[1]] c = a*b;
		if(all(declassify(c) == 18446744073709551606)){Success();}else{Failure("Expected 18446744073709551606");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= INT8_MAX-1; pd_a3p int8[[1]] b (15)= 5; pd_a3p int8[[1]] c = a*b;
		if(all(declassify(c) == 118)){Success();}else{Failure("Expected 118");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= INT16_MAX-1; pd_a3p int16[[1]] b (15)= 5; pd_a3p int16[[1]] c = a*b;
		if(all(declassify(c) == 32758)){Success();}else{Failure("Expected 32758");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= INT32_MAX-1; pd_a3p int32[[1]] b (15)= 5; pd_a3p int32[[1]] c = a*b;
		if(all(declassify(c) == 2147483638)){Success();}else{Failure("Expected 2147483638");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= INT64_MAX-1; pd_a3p int[[1]] b (15)= 5; pd_a3p int[[1]] c = a*b;
		if(all(declassify(c) == 9223372036854775798)){Success();}else{Failure("Expected 9223372036854775798");}
	}
	/****************************************************************************
    *****************************************************************************
    ***********   Division with private vectors highly unstable   ***************
    ***********          use only on uint32 vectors               ***************
	*****************************************************************************
	****************************************************************************/
	print("TEST 22: Division with two private values");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; pd_a3p uint8[[1]] b (15)= 174; pd_a3p uint8[[1]] c = b/a;
		if(all(declassify(c) == 11)){Success();}else{Failure("Expected 11");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; pd_a3p uint16[[1]] b (15)= 45876; pd_a3p uint16[[1]] c = b/a;
		if(all(declassify(c) == 262)){Success();}else{Failure("Expected 262");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; pd_a3p uint32[[1]] b (15)= 21798357; pd_a3p uint32[[1]] c = b/a;
		if(all(declassify(c) == 9090)){Success();}else{Failure("Expected 9090");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; pd_a3p uint[[1]] b (15)= 1872698523698; pd_a3p uint[[1]] c = b/a;
		if(all(declassify(c) == 726146)){Success();}else{Failure("Expected 726146");}
	}
	print("TEST 23: Division with one private one public value");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 174; pd_a3p uint8[[1]] c = b/a;
		if(all(declassify(c) == 11)){Success();}else{Failure("Expected 11");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 45876; pd_a3p uint16[[1]] c = b/a;
		if(all(declassify(c) == 262)){Success();}else{Failure("Expected 262");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 21798357; pd_a3p uint32[[1]] c = b/a;
		if(all(declassify(c) == 9090)){Success();}else{Failure("Expected 9090");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 1872698523698; pd_a3p uint[[1]] c = b/a;
		if(all(declassify(c) == 726146)){Success();}else{Failure("Expected 726146");}
	}
	print("TEST 24: Division with one private one public value(2)");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 174; pd_a3p uint8[[1]] c = a/b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 45876; pd_a3p uint16[[1]] c = a/b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 21798357; pd_a3p uint32[[1]] c = a/b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 1872698523698; pd_a3p uint[[1]] c = a/b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	print("TEST 26: 0 divided with random private vectors");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 0; pd_a3p uint8[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 0; pd_a3p uint16[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 0; pd_a3p uint32[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 0; pd_a3p uint[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	print("TEST 29: Division accuracy private");
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= UINT8_MAX; pd_a3p uint8[[1]] b (15)= UINT8_MAX -1 ; pd_a3p uint8[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= UINT16_MAX; pd_a3p uint16[[1]] b (15)= UINT16_MAX -1; pd_a3p uint16[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= UINT32_MAX; pd_a3p uint32[[1]] b (15)= UINT32_MAX -1; pd_a3p uint32[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] a (15)= UINT64_MAX; pd_a3p uint[[1]] b (15)= UINT64_MAX-1; pd_a3p uint[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= INT8_MAX; pd_a3p int8[[1]] b (15)= INT8_MAX-1; pd_a3p int8[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= INT16_MAX; pd_a3p int16[[1]] b (15)= INT16_MAX-1; pd_a3p int16[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= INT32_MAX; pd_a3p int32[[1]] b (15)= INT32_MAX-1; pd_a3p int32[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int");
		pd_a3p int[[1]] a (15)= INT64_MAX; pd_a3p int[[1]] b (15)= INT64_MAX-1; pd_a3p int[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	/****************************************************************************
    *****************************************************************************
    ***********    Modulo for private vectors implemented but     ***************
    ***********       not working even with uint32 vectors        ***************
	*****************************************************************************
	****************************************************************************/
	// as a sidenote, using modulo on any private value except uint32 will stop miners 2 and 3
	// and freeze miner 1
	// as for uint32, it will give a SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE
	print("TEST 31: Modulo private vectors");
	{
		print("uint8");
		pd_a3p uint8[[1]] b (15)= 15; pd_a3p uint8[[1]] a (15)= 174; pd_a3p uint8[[1]] c = a%b;
		if(all(declassify(c) == 9)){Success();}else{Failure("Expected 9");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] b (15)= 175; pd_a3p uint16[[1]] a (15)= 45876; pd_a3p uint16[[1]] c = a%b;
		if(all(declassify(c) == 26)){Success();}else{Failure("Expected 26");}
	}
	{
		// unlike other data types, uint32 throws SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE
		// other data types stop miners 2 and 3;
		print("uint32");
		pd_a3p uint32[[1]] b (15)= 2398; pd_a3p uint32[[1]] a (15)= 21798357; pd_a3p uint32[[1]] c = a%b;
		if(all(declassify(c) == 537)){Success();}else{Failure("Expected 537");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] b (15)= 2578953; pd_a3p uint[[1]] a (15)= 1872698523698; pd_a3p uint[[1]] c = a%b;
		if(all(declassify(c) == 2118560)){Success();}else{Failure("Expected 2118560");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] b (15)= -25; pd_a3p int8[[1]] a (15)= 50; pd_a3p int8[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] b (15)= -2264; pd_a3p int16[[1]] a (15)= 22468; pd_a3p int16[[1]] c = a%b;
		if(all(declassify(c) == 2092)){Success();}else{Failure("Expected 2092");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] b (15)= -12549; pd_a3p int32[[1]] a (15)= 21485747; pd_a3p int32[[1]] c = a%b;
		if(all(declassify(c) == 1859)){Success();}else{Failure("Expected 1859");}
	}
	{
		print("int");
		pd_a3p int[[1]] b (15)= 2954; pd_a3p int[[1]] a (15)= 93214654775807; pd_a3p int[[1]] c = a%b;
		if(all(declassify(c) == 257)){Success();}else{Failure("Expected 257");}
	}
	print("TEST 32: Modulo with private and public vectors");
	{
		print("uint8");
		pd_a3p uint8[[1]] b (15)= 15; uint8[[1]] a (15)= 174; pd_a3p uint8[[1]] c = a%b;
		if(all(declassify(c) == 9)){Success();}else{Failure("Expected 9");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] b (15)= 175; uint16[[1]] a (15)= 45876; pd_a3p uint16[[1]] c = a%b;
		if(all(declassify(c) == 26)){Success();}else{Failure("Expected 26");}
	}
	{
		// unlike other data types, uint32 throws SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE
		// other data types stop miners 2 and 3;
		print("uint32");
		pd_a3p uint32[[1]] b (15)= 2398; uint32[[1]] a (15)= 21798357; pd_a3p uint32[[1]] c = a%b;
		if(all(declassify(c) == 537)){Success();}else{Failure("Expected 537");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] b (15)= 2578953; uint[[1]] a (15)= 1872698523698; pd_a3p uint[[1]] c = a%b;
		if(all(declassify(c) == 2118560)){Success();}else{Failure("Expected 2118560");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] b (15)= -25; int8[[1]] a (15)= 50; pd_a3p int8[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] b (15)= -2264; int16[[1]] a (15)= 22468; pd_a3p int16[[1]] c = a%b;
		if(all(declassify(c) == 2092)){Success();}else{Failure("Expected 2092");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] b (15)= -12549; int32[[1]] a (15)= 21485747; pd_a3p int32[[1]] c = a%b;
		if(all(declassify(c) == 1859)){Success();}else{Failure("Expected 1859");}
	}
	{
		print("int");
		pd_a3p int[[1]] b (15)= 2954; int[[1]] a (15)= 93214654775807; pd_a3p int[[1]] c = a%b;
		if(all(declassify(c) == 257)){Success();}else{Failure("Expected 257");}
	}
	print("TEST 33: Modulo with private and public vectors(2)");
	{
		print("uint8");
		pd_a3p uint8[[1]] b (15)= 174; uint8[[1]] a (15)= 15; pd_a3p uint8[[1]] c = b%a;
		if(all(declassify(c) == 9)){Success();}else{Failure("Expected 9");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] b (15)= 45876; uint16[[1]] a (15)= 175 ; pd_a3p uint16[[1]] c = b%a;
		if(all(declassify(c) == 26)){Success();}else{Failure("Expected 26");}
	}
	{
		// unlike other data types, uint32 throws SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE
		// other data types stop miners 2 and 3;
		print("uint32");
		pd_a3p uint32[[1]] b (15)= 21798357; uint32[[1]] a (15)= 2398 ; pd_a3p uint32[[1]] c = b%a;
		if(all(declassify(c) == 537)){Success();}else{Failure("Expected 537");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] b (15)= 1872698523698; uint[[1]] a (15)= 2578953; pd_a3p uint[[1]] c = b%a;
		if(all(declassify(c) == 2118560)){Success();}else{Failure("Expected 2118560");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] b (15)= 50; int8[[1]] a (15)= -25; pd_a3p int8[[1]] c = b%a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] b (15)= 22468; int16[[1]] a (15)= -2264; pd_a3p int16[[1]] c = b%a;
		if(all(declassify(c) == 2092)){Success();}else{Failure("Expected 2092");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] b (15)= 21485747; int32[[1]] a (15)= -12549; pd_a3p int32[[1]] c = b%a;
		if(all(declassify(c) == 1859)){Success();}else{Failure("Expected 1859");}
	}
	{
		print("int");
		pd_a3p int[[1]] b (15)= 93214654775807; int[[1]] a (15)= 2954; pd_a3p int[[1]] c = b%a;
		if(all(declassify(c) == 257)){Success();}else{Failure("Expected 257");}
	}
	print("TEST 34: Modulo with important private vectors");
	{
		print("uint8");
		pd_a3p uint8[[1]] b (15)= 1; pd_a3p uint8[[1]] a (15)= 0; pd_a3p uint8[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = 1;  a = 1;  c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = UINT8_MAX;  a = UINT8_MAX;  c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] b (15)= 1; pd_a3p uint16[[1]] a (15)= 0; pd_a3p uint16[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = 1;  a = 1; c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = UINT16_MAX;a = UINT16_MAX;c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		// unlike other data types, uint32 throws SHAREMIND_VM_PROCESS_GENERAL_SYSCALL_FAILURE
		// other data types stop miners 2 and 3;
		print("uint32");
		pd_a3p uint32[[1]] b (15)= 1; pd_a3p uint32[[1]] a (15)= 0; pd_a3p uint32[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = 1; a = 1; c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = UINT32_MAX; a = UINT32_MAX; c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint");
		pd_a3p uint[[1]] b (15)= 1; pd_a3p uint[[1]] a (15)= 0; pd_a3p uint[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = 1; a = 1;c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = UINT64_MAX; a = UINT64_MAX;  c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] b (15)= 1; pd_a3p int8[[1]] a (15)= 0; pd_a3p int8[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = 1;  a = 1;  c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = INT8_MAX;  a = INT8_MAX;  c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] b (15)= 1; pd_a3p int16[[1]] a (15)= 0; pd_a3p int16[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = 1;a = 1;c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = INT16_MAX;a = INT16_MAX;c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] b (15)= 1; pd_a3p int32[[1]] a (15)= 0; pd_a3p int32[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = 1;a = 1;c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = INT32_MAX;a = INT32_MAX;c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int");
		pd_a3p int[[1]] b (15)= 1; pd_a3p int[[1]] a (15)= 0; pd_a3p int[[1]] c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = 1;a = 1;c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}

		b = INT64_MAX;a = INT64_MAX;c = a%b;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0");}
	}
	print("TEST 38: private boolean negation (!)");
	{
		print("bool");
		pd_a3p bool[[1]] a (15)= true;
		pd_a3p bool[[1]] b (15)= false;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}

		if(all(declassify(b != a))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("uint8");
		pd_a3p uint8[[1]] a (15)= 25;
		pd_a3p uint8[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] a (15)= 25;
		pd_a3p uint16[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] a (15)= 25;
		pd_a3p uint32[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("uint64");
		pd_a3p uint64[[1]] a (15)= 25;
		pd_a3p uint64[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("int8");
		pd_a3p int8[[1]] a (15)= 25;
		pd_a3p int8[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("int16");
		pd_a3p int16[[1]] a (15)= 25;
		pd_a3p int16[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("int32");
		pd_a3p int32[[1]] a (15)= 25;
		pd_a3p int32[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("int64/int");
		pd_a3p int64[[1]] a (15)= 25;
		pd_a3p int64[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8[[1]] a (15)= 25;
		pd_a3p xor_uint8[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16[[1]] a (15)= 25;
		pd_a3p xor_uint16[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32[[1]] a (15)= 25;
		pd_a3p xor_uint32[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64[[1]] a (15)= 25;
		pd_a3p xor_uint64[[1]] b (15)= 26;
		if(all(declassify(a != b))){Success();}
		else{Failure("Boolean negation failed");}
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);

    test_report(all_tests, succeeded_tests);
}
