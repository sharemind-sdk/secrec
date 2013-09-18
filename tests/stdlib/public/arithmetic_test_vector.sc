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

	print("TEST 1: Addition with two public vectors");
	{
		print("uint8");
		uint8[[1]] a (15) = 15; uint8[[1]] b (15)= 174 ; uint8[[1]] c = a+b;
		if(all(c == 189)){Success();}else{Failure("Expected 189");}
	}
	{
		print("uint16");
		uint16[[1]] a (15) = 175; uint16[[1]] b (15)= 45876; uint16[[1]] c = a+b;
		if(all(c == 46051)){Success();}else{Failure("Expected 46051");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 21798357; uint32[[1]] c = a+b;
		if(all(c == 21800755)){Success();}else{Failure("Expected 21800755");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 1872698523698; uint[[1]] c = a+b;
		if(all(c == 1872701102651)){Success();}else{Failure("Expected 1872701102651");}
	}
	{
		print("int8");
		int8[[1]] a (15)= -25; int8[[1]] b (15)= 50; int8[[1]] c = a+b;
		if(all(c == 25)){Success();}else{Failure("Expected 25");}
	}
	{
		print("int16");
		int16[[1]] a (15)= -2264; int16[[1]] b (15)= 22468; int16[[1]] c = a+b;
		if(all(c == 20204)){Success();}else{Failure("Expected 20204");}
	}
	{
		print("int32");
		int32[[1]] a (15)= -12549; int32[[1]] b (15)= 21485747; int32[[1]] c = a+b;
		if(all(c == 21473198)){Success();}else{Failure("Expected 21800755");}
	}
	{
		print("int");
		int[[1]] a (15)= 2954; int[[1]] b (15)= 93214654775807; int[[1]] c = a+b;
		if(all(c == 93214654778761)){Success();}else{Failure("Expected 93214654778761");}
	}
	print("TEST 5: Addition with two public vectors modulo (type_MAX + 1)");
	{
		print("uint8");
		uint8[[1]] a (15)= 1; uint8[[1]] b (15)= UINT8_MAX; uint8[[1]] c = a+b;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 1; uint16[[1]] b (15)= UINT16_MAX; uint16[[1]] c = a+b;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 1; uint32[[1]] b (15)= UINT32_MAX; uint32[[1]] c = a+b;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 1; uint[[1]] b (15)= UINT64_MAX; uint[[1]] c = a+b;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 1; int8[[1]] b (15)= INT8_MAX; int8[[1]] c = a+b;
		if(all(c == INT8_MIN)){Success();}else{Failure("Expected -128");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 1; int16[[1]] b (15)= INT16_MAX; int16[[1]] c = a+b;
		if(all(c == INT16_MIN)){Success();}else{Failure("Expected -32768");}
	}
	{
		print("int32");
		int32[[1]] a (15)= 1; int32[[1]] b (15)= INT32_MAX; int32[[1]] c = a+b;
		if(all(c == INT32_MIN)){Success();}else{Failure("Expected -2147483648");}
	}
	{
		print("int");
		int[[1]] a (15)= 1; int[[1]] b (15)= INT64_MAX; int[[1]] c = a+b;
		if(all(c == INT64_MIN)){Success();}else{Failure("Expected -9223372036854775808");}
	}
	print("TEST 7: Addition with public vectors modulo (A + T_MAX+1 = A)");
	{
		print("uint8");
		uint8[[1]] a (15)= 15; uint8[[1]] b (15)= UINT8_MAX; uint8[[1]] c = a+b+1;
		if(all(c == a)){Success();}else{Failure("Expected 15");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 175; uint16[[1]] b (15)= UINT16_MAX; uint16[[1]] c = a+b+1;
		if(all(c == a)){Success();}else{Failure("Expected 175");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= UINT32_MAX; uint32[[1]] c = a+b+1;
		if(all(c == a)){Success();}else{Failure("Expected 2398");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 2578953; uint[[1]] b (15)= UINT64_MAX; uint[[1]] c = a+b+1;
		if(all(c == a)){Success();}else{Failure("Expected 2578953");}
	}
	print("TEST 9: Subtraction with two public vectors");
	{
		print("uint8");
		uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 174; uint8[[1]] c = b-a;
		if(all(c == 159)){Success();}else{Failure("Expected 159");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 45876; uint16[[1]] c = b-a;
		if(all(c == 45701)){Success();}else{Failure("Expected 46051");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 2398; uint32[[1]] b (15) = 21798357; uint32[[1]] c = b-a;
		if(all(c == 21795959)){Success();}else{Failure("Expected 21795959");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 1872698523698; uint[[1]] c = b-a;
		if(all(c == 1872695944745)){Success();}else{Failure("Expected 1872695944745");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 25; int8[[1]] b (15)= 50; int8[[1]] c = b-a;
		if(all(c == 25)){Success();}else{Failure("Expected 25");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 42264; int16[[1]] b (15)= 22468; int16[[1]] c = b-a;
		if(all(c == -19796)){Success();}else{Failure("Expected -19796");}
	}
	{
		print("int32");
		int32[[1]] a (15)= 12549; int32[[1]] b (15)= 21485747; int32[[1]] c = b-a;
		if(all(c == 21473198)){Success();}else{Failure("Expected 21473198");}
	}
	{
		print("int");
		int[[1]] a (15)= 2954; int[[1]] b (15) = 93214654775807; int[[1]] c = b-a;
		if(all(c == 93214654772853)){Success();}else{Failure("Expected 93214654772853");}
	}
	print("TEST 13: Subtraction with two public vectors modulo (type_MIN - 1)");
	{
		print("uint8");
		uint8[[1]] a (15)= 1; uint8[[1]] b (15)= 0; uint8[[1]] c = b-a;
		if(all(c == UINT8_MAX)){Success();}else{Failure("Expected 255");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 1; uint16[[1]] b (15)= 0; uint16[[1]] c = b-a;
		if(all(c == UINT16_MAX)){Success();}else{Failure("Expected 65535");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 1; uint32[[1]] b (15)= 0; uint32[[1]] c = b-a;
		if(all(c == UINT32_MAX)){Success();}else{Failure("Expected 4294967295");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 1; uint[[1]] b (15)= 0; uint[[1]] c = b-a;
		if(all(c == UINT64_MAX)){Success();}else{Failure("Expected 18446744073709551615");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 1; int8[[1]] b (15)= INT8_MIN; int8[[1]] c = b-a;
		if(all(c == INT8_MAX)){Success();}else{Failure("Expected 127");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 1; int16[[1]] b (15)= INT16_MIN; int16[[1]] c = b-a;
		if(all(c == INT16_MAX)){Success();}else{Failure("Expected 32767");}
	}
	{
		print("int32");
		int32[[1]] a (15)= 1; int32[[1]] b (15)= INT32_MIN; int32[[1]] c = b-a;
		if(all(c == INT32_MAX)){Success();}else{Failure("Expected 2147483647");}
	}
	{
		print("int");
		int[[1]] a = 1; int[[1]] b = INT64_MIN; int[[1]] c = b-a;
		if(all(c == INT64_MAX)){Success();}else{Failure("Expected 9223372036854775807");}
	}
	print("TEST 15: Multiplication with two public vectors");
	{
		print("uint8");
		uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 12; uint8[[1]] c = a*b;
		if(all(c == 180)){Success();}else{Failure("Expected 180");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 139; uint16[[1]] c = a*b;
		if(all(c == 24325)){Success();}else{Failure("Expected 24325");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 4051; uint32[[1]] c = a*b;
		if(all(c == 9714298)){Success();}else{Failure("Expected 9714298");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 248924; uint[[1]] b (15)= 48265; uint[[1]] c = a*b;
		if(all(c == 12014316860)){Success();}else{Failure("Expected 12014316860");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 25; int8[[1]] b (15)= -4; int8[[1]] c = a*b;
		if(all(c == -100)){Success();}else{Failure("Expected -100");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 175; int16[[1]] b (15)= 139; int16[[1]] c = a*b;
		if(all(c == 24325)){Success();}else{Failure("Expected 24325");}
	}
	{
		print("int32");
		int32[[1]] a (15)= -2398; int32[[1]] b (15)= 4051; int32[[1]] c = a*b;
		if(all(c == -9714298)){Success();}else{Failure("Expected -9714298");}
	}
	{
		print("int");
		int[[1]] a (15)= 248924; int[[1]] b (15)= 48265; int[[1]] c = a*b;
		if(all(c == 12014316860)){Success();}else{Failure("Expected 12014316860");}
	}
	print("TEST 19: Multiplication with two public vectors modulo (type_MAX * 5)");
	{
		print("uint8");
		uint8[[1]] a (15)= UINT8_MAX-1; uint8[[1]] c = a*5;
		if(all(c == 246)){Success();}else{Failure("Expected 246");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= UINT16_MAX-1; uint16[[1]] c = a*5;
		if(all(c == 65526)){Success();}else{Failure("Expected 65526");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= UINT32_MAX-1; uint32[[1]] c = a*5;
		if(all(c == 4294967286)){Success();}else{Failure("Expected 4294967286");}
	}
	{
		print("uint");
		uint[[1]] a (15)= UINT64_MAX-1; uint[[1]] c = a*5;
		if(all(c == 18446744073709551606)){Success();}else{Failure("Expected 18446744073709551606");}
	}
	{
		print("int8");
		int8[[1]] a (15)= INT8_MAX-1; int8[[1]] c = a*5;
		if(all(c == 118)){Success();}else{Failure("Expected 118");}
	}
	{
		print("int16");
		int16[[1]] a (15)= INT16_MAX-1; int16[[1]] c = a*5;
		if(all(c == 32758)){Success();}else{Failure("Expected 32758");}
	}
	{
		print("int32");
		int32[[1]] a (15)= INT32_MAX-1; int32[[1]] c = a*5;
		if(all(c == 2147483638)){Success();}else{Failure("Expected 2147483638");}
	}
	{
		print("int");
		int[[1]] a (15)= INT64_MAX-1; int[[1]] c = a*5;
		if(all(c == 9223372036854775798)){Success();}else{Failure("Expected 9223372036854775798");}
	}
	/****************************************************************************
    *****************************************************************************
    ***********   Division with private vectors highly unstable   ***************
    ***********          use only on uint32 vectors               ***************
	*****************************************************************************
	****************************************************************************/
	print("TEST 21: Division with two public vectors");
	{
		print("uint8");
		uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 174; uint8[[1]] c = b/a;
		if(all(c == 11)){Success();}else{Failure("Expected 11");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 45876; uint16[[1]] c = b/a;
		if(all(c == 262)){Success();}else{Failure("Expected 262");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 21798357; uint32[[1]] c = b/a;
		if(all(c == 9090)){Success();}else{Failure("Expected 9090");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 1872698523698; uint[[1]] c = b/a;
		if(all(c == 726146)){Success();}else{Failure("Expected 726146");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 25; int8[[1]] b (15)= 50; int8[[1]] c = b/a;
		if(all(c == 2)){Success();}else{Failure("Expected 2");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 42264; int16[[1]] b (15)= 22468; int16[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int32");
		int32[[1]] a (15)= 12549; int32[[1]] b (15)= 21485747; int32[[1]] c = b/a;
		if(all(c == 1712)){Success();}else{Failure("Expected 1712");}
	}
	{
		print("int");
		int[[1]] a (15)= 982175129826; int[[1]] b (15)= 93214654775807; int[[1]] c = b/a;
		if(all(c == 94)){Success();}else{Failure("Expected 94");}
	}
	print("TEST 25: 0 divided with random public vectors");
	{
		print("uint8");
		uint8[[1]] a (15)= 15; uint8[[1]] b (15)= 0; uint8[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 175; uint16[[1]] b (15)= 0; uint16[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 2398; uint32[[1]] b (15)= 0; uint32[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 2578953; uint[[1]] b (15)= 0; uint[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 25; int8[[1]] b (15)= 0; int8[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 42264; int16[[1]] b (15)= 0; int16[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int32");
		int32[[1]] a (15)= 12549; int32[[1]] b (15)= 0; int32[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int");
		int[[1]] a (15)= 982175129826; int[[1]] b (15)= 0; int[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	print("TEST 27: A/A = 1 with all public types");
	{
		print("uint8");
		uint8[[1]] a (15)= 174; uint8[[1]] b (15)= 174; uint8[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 45876; uint16[[1]] b (15)= 45876; uint16[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 21798357; uint32[[1]] b (15)= 21798357; uint32[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 1872698523698; uint[[1]] b (15)= 1872698523698; uint[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 50; int8[[1]] b (15)= 50; int8[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 22468; int16[[1]] b (15)= 22468; int16[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1");}
	}
	{
		print("int32");
		int32[[1]] a (15)= 21485747; int32[[1]] b (15)= 21485747; int32[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1");}
	}
	{
		print("int");
		int[[1]] a (15)= 93214654775807; int[[1]] b (15)= 93214654775807; int[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1");}
	}
	print("TEST 28: Division accuracy public");
	{
		print("uint8");
		uint8[[1]] a (15)= UINT8_MAX; uint8[[1]] b (15)= UINT8_MAX -1 ; uint8[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= UINT16_MAX; uint16[[1]] b (15)= UINT16_MAX -1; uint16[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= UINT32_MAX; uint32[[1]] b (15)= UINT32_MAX -1; uint32[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("uint");
		uint[[1]] a (15)= UINT64_MAX; uint[[1]] b (15)= UINT64_MAX-1; uint[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int8");
		int8[[1]] a (15)= INT8_MAX; int8[[1]] b (15)= INT8_MAX-1; int8[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int16");
		int16[[1]] a (15)= INT16_MAX; int16[[1]] b (15)= INT16_MAX-1; int16[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int32");
		int32[[1]] a (15)= INT32_MAX; int32[[1]] b (15)= INT32_MAX-1; int32[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int");
		int[[1]] a (15)= INT64_MAX; int[[1]] b (15)= INT64_MAX-1; int[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
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
	print("TEST 30: Modulo on public vectors");
	{
		print("uint8");
		uint8[[1]] b (15)= 15; uint8[[1]] a (15)= 174; uint8[[1]] c = a%b;
		if(all(c == 9)){Success();}else{Failure("Expected 9");}
	}
	{
		print("uint16");
		uint16[[1]] b (15)= 175; uint16[[1]] a (15)= 45876; uint16[[1]] c = a%b;
		if(all(c == 26)){Success();}else{Failure("Expected 26");}
	}
	{
		print("uint32");
		uint32[[1]] b (15)= 2398; uint32[[1]] a (15)= 21798357; uint32[[1]] c = a%b;
		if(all(c == 537)){Success();}else{Failure("Expected 537");}
	}
	{
		print("uint");
		uint[[1]] b (15)= 2578953; uint[[1]] a (15)= 1872698523698; uint[[1]] c = a%b;
		if(all(c == 2118560)){Success();}else{Failure("Expected 2118560");}
	}
	{
		print("int8");
		int8[[1]] b (15)= -25; int8[[1]] a (15)= 50; int8[[1]] c = a%b;
		if(all(c == 0)){Success();}else{Failure("Expected 0");}
	}
	{
		print("int16");
		int16[[1]] b (15)= -2264; int16[[1]] a (15)= 22468; int16[[1]] c = a%b;
		if(all(c == 2092)){Success();}else{Failure("Expected 2092");}
	}
	{
		print("int32");
		int32[[1]] b (15)= -12549; int32[[1]] a (15)= 21485747; int32[[1]] c = a%b;
		if(all(c == 1859)){Success();}else{Failure("Expected 1859");}
	}
	{
		print("int");
		int[[1]] b (15)= 2954; int[[1]] a (15)= 93214654775807; int[[1]] c = a%b;
		if(all(c == 257)){Success();}else{Failure("Expected 257");}
	}
	print("TEST 35: Operation priorities : Multiplication over addition");
	{
		print("uint8");
		uint8[[1]] a (15)= 5; uint8[[1]] b (15)= 20; uint8[[1]] c (15)= 45;
		uint8[[1]] d (15)= c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 5; uint16[[1]] b (15)= 20; uint16[[1]] c (15)= 45;
		uint16[[1]] d = c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 5; uint32[[1]] b (15)= 20; uint32[[1]] c (15)= 45;
		uint32[[1]] d = c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 5; uint[[1]] b (15)= 20; uint[[1]] c (15)= 45;
		uint[[1]] d = c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 5; int8[[1]] b (15)= 20; int8[[1]] c (15)= 45;
		int8[[1]] d = c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 5; int16[[1]] b (15)= 20; int16[[1]] c (15)= 45;
		int16[[1]] d = c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145");}
	}
	{
		print("int32");
		uint32[[1]] a (15)= 5; uint32[[1]] b (15)= 20; uint32[[1]] c (15)= 45;
		uint32[[1]] d = c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145");}
	}
	{
		print("int");
		int[[1]] a (15)= 5; int[[1]] b (15)= 20; int[[1]] c (15)= 45;
		int[[1]] d = c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145");}
	}
	print("TEST 36: Operation priorities : Parentheses over multiplication");
	{
		print("uint8");
		uint8[[1]] a (15)= 5; uint8[[1]] b (15)= 5; uint8[[1]] c (15)= 20;
		uint8[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 5; uint16[[1]] b (15)= 5; uint16[[1]] c (15)= 20;
		uint16[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 5; uint32[[1]] b (15)= 5; uint32[[1]] c (15)= 20;
		uint32[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 5; uint[[1]] b (15)= 5; uint[[1]] c (15)= 20;
		uint[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 5; int8[[1]] b (15)= 5; int8[[1]] c (15)= 20;
		int8[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 5; int16[[1]] b (15)= 5; int16[[1]] c (15)= 20;
		int16[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125");}
	}
	{
		print("int32");
		uint32[[1]] a (15)= 5; uint32[[1]] b (15)= 5; uint32[[1]] c (15)= 20;
		uint32[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125");}
	}
	{
		print("int");
		int[[1]] a (15)= 5; int[[1]] b (15)= 5; int[[1]] c (15)= 20;
		int[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125");}
	}
	print("TEST 37: Operation priorities : Division over addition and subtraction");
	{
		print("uint8");
		uint8[[1]] a (15)= 5; uint8[[1]] b (15)= 5; uint8[[1]] c (15)= 20; uint8[[1]] d (15)= 5;
		uint8[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 5; uint16[[1]] b (15)= 5; uint16[[1]] c (15)= 20; uint16[[1]] d (15)= 5;
		uint16[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 5; uint32[[1]] b (15)= 5; uint32[[1]] c (15)= 20; uint32[[1]] d (15)= 5;
		uint32[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16");}
	}
	{
		print("uint");
		uint[[1]] a (15)= 5; uint[[1]] b (15)= 5; uint[[1]] c (15)= 20; uint[[1]] d (15)= 5;
		uint[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 5; int8[[1]] b (15)= 5; int8[[1]] c (15)= 20; int8[[1]] d (15)= 5;
		int8[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 5; int16[[1]] b (15)= 5; int16[[1]] c (15)= 20; int16[[1]] d (15)= 5;
		int16[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16");}
	}
	{
		print("int32");
		int32[[1]] a (15)= 5; int32[[1]] b (15)= 5; int32[[1]] c (15)= 20; int32[[1]] d (15)= 5;
		int32[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16");}
	}
	{
		print("int");
		int[[1]] a (15)= 5; int[[1]] b (15)= 5; int[[1]] c (15)= 20; int[[1]] d (15)= 5;
		int[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16");}
	}

	print("TEST 38: public boolean negation (!)");
	{
		print("bool");
		bool[[1]] a (15)= true;
		if(all(a != false)){Success();}
		else{Failure("Boolean negation failed");}
		a = false;
		if(all(a != true)){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("uint8");
		uint8[[1]] a (15)= 25;
		uint8[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("uint16");
		uint16[[1]] a (15)= 25;
		uint16[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("uint32");
		uint32[[1]] a (15)= 25;
		uint32[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("uint64");
		uint64[[1]] a (15)= 25;
		uint64[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("int8");
		int8[[1]] a (15)= 25;
		int8[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("int16");
		int16[[1]] a (15)= 25;
		int16[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("int32");
		int32[[1]] a (15)= 25;
		int32[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negation failed");}
	}
	{
		print("int64/int");
		int64[[1]] a (15)= 25;
		int64[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negation failed");}
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);

    test_report(all_tests, succeeded_tests);
}
