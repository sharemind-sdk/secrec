/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module comparisons_test_public_vectors;

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

domain pd_a3p additive3pp;

template<type T>
void larger_than(T[[1]] vec,T[[1]] vec2){
	if(any(vec2 > vec)){
		print("FAILURE! > operator malfunctioning");
 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<type T>
void smaller_than(T[[1]] vec,T[[1]] vec2){
	if(any(vec2 < vec)){
		print("FAILURE! < operator malfunctioning");
 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<type T>
void larger_than_equal(T[[1]] vec,T[[1]] vec2){
	if(any(vec2 >= vec)){
		print("FAILURE! >= operator malfunctioning");
 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<type T>
void smaller_than_equal(T[[1]] vec,T[[1]] vec2){
	if(any(vec2 <= vec)){
		print("FAILURE! <= operator malfunctioning");
 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<type T>
void equal_equal(T[[1]] vec,T[[1]] vec2){
	if(all(vec2 == vec)){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! == operator malfunctioning");
 		all_tests = all_tests +1;
	}
}


void main(){
	print("Comparisons test: start");

	print("TEST 1: > operator");
	{
		print("bool");
		bool[[1]] vec (5) = true;
		bool[[1]] vec2 (5) = false;
		larger_than(vec,vec2);
	}
	{
		print("uint8");
		uint8[[1]] vec (4)= {90,170,254,255};
		uint8[[1]] vec2 (4)= {60,169,253,1}; 
		larger_than(vec,vec2);
	}
	{
		print("uint16");
		uint16[[1]] vec (4)= {2000,20786,51935,65535};
		uint16[[1]] vec2 (4)= {2000,20785,0,32658}; 
		larger_than(vec,vec2);	
	}
	{
		print("uint32");
		uint32[[1]] vec (4)= {98274,12897533,219857621,4294967295};
		uint32[[1]] vec2 (4)= {98273,11276456,0,4294967295}; 
		larger_than(vec,vec2);	
	}
	{	
		print("uint64/uint");
		uint[[1]] vec (4)= {21576921865219,72198462159826,19562189521758219,18446744073709551615};
		uint[[1]] vec2 (4)= {21576921865218,0,19562189521758218,674789216591269}; 
		larger_than(vec,vec2);	
	}
	{
		print("int8");
		int8[[1]] vec (6)= {127,-128,1,-1,-50,50};
		int8[[1]] vec2 (6)= {126,-128,-35,-62,-89,-25}; 
		larger_than(vec,vec2);	
	}
	{
		print("int16");
		int16[[1]] vec (6)= {32767,-32767,1,-1,-12856,2389};
		int16[[1]] vec2 (6)= {32766,-32768,-1,-8295,-32768,2189}; 
		larger_than(vec,vec2);	
	}
	{
		print("int32");
		int32[[1]] vec (6)= {2147483647,-2147483648,1298749,-129857219,-1643872691,1298573027};
		int32[[1]] vec2 (6)= {2147483646,-2147483648,0,-218947828,-1789267491,-12895}; 
		larger_than(vec,vec2);
	}
	{
		print("int64/int");
		int[[1]] vec (6)= {9223372036854775807,-9223372036854775808,1,-1,-52398573219712,193286529385326552};
		int[[1]] vec2 (6)= {2198742159862154,-9223372036854775808,-92903276592,-129832598,-52398573229828,-218923896723}; 
		larger_than(vec,vec2);
	}
	print("TEST 2: < operator");
	{
		print("bool");
		bool[[1]] vec (5) = false;
		bool[[1]] vec2 (5) = true;
		smaller_than(vec,vec2);
	}
		{
		print("uint8");
		uint8[[1]] vec (4)= {90,170,254,255};
		uint8[[1]] vec2 (4)= {60,169,253,1}; 
		smaller_than(vec2,vec);
	}
	{
		print("uint16");
		uint16[[1]] vec (4)= {2000,20786,51935,65535};
		uint16[[1]] vec2 (4)= {2000,20785,0,32658}; 
		smaller_than(vec2,vec);
	}
	{
		print("uint32");
		uint32[[1]] vec (4)= {98274,12897533,219857621,4294967295};
		uint32[[1]] vec2 (4)= {98273,11276456,0,4294967295}; 
		smaller_than(vec2,vec);	
	}
	{	
		print("uint64/uint");
		uint[[1]] vec (4)= {21576921865219,72198462159826,19562189521758219,18446744073709551615};
		uint[[1]] vec2 (4)= {21576921865218,0,19562189521758218,674789216591269}; 
		smaller_than(vec2,vec);	
	}
	{
		print("int8");
		int8[[1]] vec (6)= {127,-128,1,-1,-50,50};
		int8[[1]] vec2 (6)= {126,-128,-35,-62,-89,-25}; 
		smaller_than(vec2,vec);	
	}
	{
		print("int16");
		int16[[1]] vec (6)= {32767,-32767,1,-1,-12856,2389};
		int16[[1]] vec2 (6)= {32766,-32768,-1,-8295,-32768,2189}; 
		smaller_than(vec2,vec);	
	}
	{
		print("int32");
		int32[[1]] vec (6)= {2147483647,-2147483648,1298749,-129857219,-1643872691,1298573027};
		int32[[1]] vec2 (6)= {2147483646,-2147483648,0,-218947828,-1789267491,-12895}; 
		smaller_than(vec2,vec);
	}
	{
		print("int64/int");
		int[[1]] vec (6)= {9223372036854775807,-9223372036854775808,1,-1,-52398573219712,193286529385326552};
		int[[1]] vec2 (6)= {2198742159862154,-9223372036854775808,-92903276592,-129832598,-52398573229828,-218923896723}; 
		smaller_than(vec2,vec);
	}
	print("TEST 3: >= operator");
	{
		print("bool");
		bool[[1]] vec (5) = true;
		bool[[1]] vec2 (5) = false;
		larger_than_equal(vec,vec2);
	}
	{
		print("uint8");
		uint8[[1]] vec (4)= {90,170,254,255};
		uint8[[1]] vec2 (4)= {60,169,253,1}; 
		larger_than_equal(vec,vec2);
	}
	{
		print("uint16");
		uint16[[1]] vec (4)= {2000,20786,51935,65535};
		uint16[[1]] vec2 (4)= {1999,20785,0,32658}; 
		larger_than_equal(vec,vec2);	
	}
	{
		print("uint32");
		uint32[[1]] vec (4)= {98274,12897533,219857621,4294967295};
		uint32[[1]] vec2 (4)= {98273,11276456,0,4294967294}; 
		larger_than_equal(vec,vec2);	
	}
	{	
		print("uint64/uint");
		uint[[1]] vec (4)= {21576921865219,72198462159826,19562189521758219,18446744073709551615};
		uint[[1]] vec2 (4)= {21576921865218,0,19562189521758218,674789216591269}; 
		larger_than_equal(vec,vec2);	
	}
	{
		print("int8");
		int8[[1]] vec (6)= {127,-127,1,-1,-50,50};
		int8[[1]] vec2 (6)= {126,-128,-35,-62,-89,-25}; 
		larger_than_equal(vec,vec2);	
	}
	{
		print("int16");
		int16[[1]] vec (6)= {32767,-32767,1,-1,-12856,2389};
		int16[[1]] vec2 (6)= {32766,-32768,-1,-8295,-32768,2189}; 
		larger_than_equal(vec,vec2);	
	}
	{
		print("int32");
		int32[[1]] vec (6)= {2147483647,-2147483647,1298749,-129857219,-1643872691,1298573027};
		int32[[1]] vec2 (6)= {2147483646,-2147483648,0,-218947828,-1789267491,-12895}; 
		larger_than_equal(vec,vec2);
	}
	{
		print("int64/int");
		int[[1]] vec (6)= {9223372036854775807,-9223372036854775807,1,-1,-52398573219712,193286529385326552};
		int[[1]] vec2 (6)= {2198742159862154,-9223372036854775808,-92903276592,-129832598,-52398573229828,-218923896723}; 
		larger_than_equal(vec,vec2);
	}
	print("TEST 4: <= operator");
	{
		print("bool");
		bool[[1]] vec (5) = false;
		bool[[1]] vec2 (5) = true;
		smaller_than_equal(vec,vec2);
	}
	{
		print("uint8");
		uint8[[1]] vec (4)= {90,170,254,255};
		uint8[[1]] vec2 (4)= {60,169,253,1}; 
		smaller_than_equal(vec2,vec);
	}
	{
		print("uint16");
		uint16[[1]] vec (4)= {2000,20786,51935,65535};
		uint16[[1]] vec2 (4)= {325,20785,0,32658}; 
		smaller_than_equal(vec2,vec);	
	}
	{
		print("uint32");
		uint32[[1]] vec (4)= {98274,12897533,219857621,4294967295};
		uint32[[1]] vec2 (4)= {98273,11276456,0,4294967294}; 
		smaller_than_equal(vec2,vec);	
	}
	{	
		print("uint64/uint");
		uint[[1]] vec (4)= {21576921865219,72198462159826,19562189521758219,18446744073709551615};
		uint[[1]] vec2 (4)= {21576921865218,0,19562189521758218,674789216591269}; 
		smaller_than_equal(vec2,vec);	
	}
	{
		print("int8");
		int8[[1]] vec (6)= {127,-127,1,-1,-50,50};
		int8[[1]] vec2 (6)= {126,-128,-35,-62,-89,-25}; 
		smaller_than_equal(vec2,vec);	
	}
	{
		print("int16");
		int16[[1]] vec (6)= {32767,-32767,1,-1,-12856,2389};
		int16[[1]] vec2 (6)= {32766,-32768,-1,-8295,-32768,2189}; 
		smaller_than_equal(vec2,vec);	
	}
	{
		print("int32");
		int32[[1]] vec (6)= {2147483647,-2147483647,1298749,-129857219,-1643872691,1298573027};
		int32[[1]] vec2 (6)= {2147483646,-2147483648,0,-218947828,-1789267491,-12895}; 
		smaller_than_equal(vec2,vec);
	}
	{
		print("int64/int");
		int[[1]] vec (6)= {9223372036854775807,-9223372036854775807,1,-1,-52398573219712,193286529385326552};
		int[[1]] vec2 (6)= {2198742159862154,-9223372036854775808,-92903276592,-129832598,-52398573229828,-218923896723}; 
		smaller_than_equal(vec2,vec);
	}

	print("TEST 5: == operator");
	{	
		print("bool");
		bool[[1]] vec (5) = true;
		bool[[1]] vec2 (5) = true;
		equal_equal(vec,vec2);
	}
	{
		print("uint8");
		uint8[[1]] vec (4)= {90,170,254,255};
		uint8[[1]] vec2 (4)= {90,170,254,255};
		equal_equal(vec,vec2);
	}
	{
		print("uint16");
		uint16[[1]] vec (4)= {2000,20786,51935,65535};
		uint16[[1]] vec2 (4)= {2000,20786,51935,65535};
		equal_equal(vec,vec2);	
	}
	{
		print("uint32");
		uint32[[1]] vec (4)= {98274,12897533,219857621,4294967295};
		uint32[[1]] vec2 (4)= {98274,12897533,219857621,4294967295};
		equal_equal(vec,vec2);	
	}
	{	
		print("uint64/uint");
		uint[[1]] vec (4)= {21576921865219,72198462159826,19562189521758219,18446744073709551615};
		uint[[1]] vec2 (4)= {21576921865219,72198462159826,19562189521758219,18446744073709551615};
		equal_equal(vec,vec2);	
	}
	{
		print("int8");
		int8[[1]] vec (6)= {127,-127,1,-1,-50,50};
		int8[[1]] vec2 (6)= {127,-127,1,-1,-50,50};
		equal_equal(vec,vec2);	
	}
	{
		print("int16");
		int16[[1]] vec (6)= {32767,-32767,1,-1,-12856,2389};
		int16[[1]] vec2 (6)= {32767,-32767,1,-1,-12856,2389};
		equal_equal(vec,vec2);
	}
	{
		print("int32");
		int32[[1]] vec (6)= {2147483647,-2147483647,1298749,-129857219,-1643872691,1298573027};
		int32[[1]] vec2 (6)= {2147483647,-2147483647,1298749,-129857219,-1643872691,1298573027};
		equal_equal(vec,vec2);
	}
	{
		print("int64/int");
		int[[1]] vec (6)= {9223372036854775807,-9223372036854775807,1,-1,-52398573219712,193286529385326552};
		int[[1]] vec2 (6)= {9223372036854775807,-9223372036854775807,1,-1,-52398573219712,193286529385326552};
		equal_equal(vec,vec2);
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}