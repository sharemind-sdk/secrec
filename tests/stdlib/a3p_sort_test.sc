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

domain pd_a3p additive3pp;

public uint32 all_tests;
public uint32 succeeded_tests;
public bool test_result;
public uint repeats = 7; // alter the range of elements to be tested for sorting (min = 4). From function test_sorting()

template <type T>
void test_sorting(T data){
	for(uint i = 3; i < repeats; ++i){
		pd_a3p T[[1]] vec (i);
		vec = randomize(vec);
		test(vec);
	}
}

template <domain D : additive3pp ,type T>
void test_sorting_xor(D T data){
	for(uint i = 3; i < repeats; ++i){
		pd_a3p T[[1]] vec (i);
		vec = randomize(vec);
		test_xor(vec);
	}
}


template <domain D : additive3pp, type T,dim N>
void test(D T[[N]] vec){
	{
		D T[[N]] vec (0);
		vec = sort(vec);
	}
	bool result = true;
	T last;
	T[[N]] vec2 = declassify(sort(vec));
	for(uint i = 0; i < size(vec);++i){
		if(i != 0){
			if(last > vec2[i]){
				result = false;
				break;
			}
			else{
				last = vec2[i];
			}
		}
		else{
			last = vec2[i];
		}
	}
	if(result){
 		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
 	}
 	else{
 		print("FAILURE! sorting process failed");
 		print("Got this : ",arrayToString(vec2), " From this: ", arrayToString(declassify(vec)));
 		all_tests = all_tests +1;
 	}
}

template <domain D : additive3pp, type T,dim N>
void test_xor(D T[[N]] vec){
	{
		D T[[N]] vec (0);
		vec = sort(vec);
	}
	bool result = true;
	D T last;
	D T[[N]] vec2 = sort(vec);
	for(uint i = 0; i < size(vec);++i){
		if(i != 0){
			if(declassify(last > vec2[i])){
				result = false;
				break;
			}
			else{
				last = vec2[i];
			}
		}
		else{
			last = vec2[i];
		}
	}
	if(result){
 		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
 	}
 	else{
 		print("FAILURE! sorting process failed");
 		print("Got this : ",arrayToString(declassify(vec2)), " From this: ", arrayToString(declassify(vec)));
 		all_tests = all_tests +1;
 	}
}


template <type T>
void test_4(T data){
	{
		pd_a3p T[[2]] mat (0,2);
		mat = sort(mat,0::uint);
		mat = sort(mat,1::uint);
	}
	pd_a3p T[[2]] mat (5,5);
	mat = randomize(mat);
	public bool result = true;
	public T last;
	public uint column;
	for(uint i = 0; i < 5; ++i){
		T[[2]] mat2 = declassify(sort(mat,i));
		for(uint j = 0; j < 5; ++j){
			if(j != 0){
				if(last > mat2[j,i]){
					result = false;
					column = i;
					break;
				}
				else{
					last = mat2[j,i];
				}
			}
			else{
				last = mat2[j,i];
			}
		}
		if(result){
	 		succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
	 	}
	 	else{
	 		print("FAILURE! sorting process failed");
	 		print("Got this : ");
	 		printMatrix(mat2);
	 		print(" From this: ");
	 		printMatrix(declassify(mat));
	 		print("While sorting column ", column);
	 		all_tests = all_tests +1;
	 	}
	}
}

template <domain D : additive3pp, type T>
void test_4_xor(D T data){
	{
		pd_a3p T[[2]] mat (0,2);
		mat = sort(mat,0::uint);
		mat = sort(mat,1::uint);
	}
	D T[[2]] mat (5,5);
	mat = randomize(mat);
	public bool result = true;
	D T last;
	public uint column;
	for(uint i = 0; i < 5; ++i){
		D T[[2]] mat2 = sort(mat,i);
		column = i;
		for(uint j = 0; j < 5; ++j){
			if(j != 0){
				if(declassify(last > mat2[j,i])){
					result = false;
					break;
				}
				else{
					last = mat2[j,i];
				}
			}
			else{
				last = mat2[j,i];
			}
		}
		if(result){
	 		succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
	 	}
	 	else{
	 		print("FAILURE! sorting process failed");
	 		print("Got this : ");
	 		printMatrix(declassify(mat2));
	 		print(" From this: ");
	 		printMatrix(declassify(mat));
	 		print("While sorting column ", column);
	 		all_tests = all_tests +1;
	 	}
	}
}

template<domain D: additive3pp, type T>
void sorting_network(D T data){
	for(uint i = 3; i < repeats; ++i){
		bool result = true;
		pd_a3p T[[1]] vec (i);
		D T last;
		vec = randomize(vec);

		printVector(declassify(vec));
		D T[[1]] vec2 = sortingNetworkSort(vec);
		printVector(declassify(vec2));

		for(uint j = 0; j < size(vec);++j){
			if(j != 0){
				if(declassify(last) > declassify(vec2[j])){
					result = false;
					break;
				}
				else{
					last = vec2[j];	
				}
			}
			else{
				last = vec2[j];
			}
		}
		
		if(!result){
			print("FAILURE! sorting process failed");
			print("Got this : ",arrayToString(declassify(vec2)), " From this: ", arrayToString(declassify(vec)));
			result = true;
			all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
			all_tests = all_tests +1;
			print("SUCCESS!");
		}
	}
}

template <domain D : additive3pp, type T>
void sorting_network_matrix(D T data){
	pd_a3p T[[2]] mat (5,5);
	mat = randomize(mat);
	public bool result = true;
	public T last;
	public uint column;
	for(uint i = 0; i < 5; ++i){
		column = i;
		T[[2]] mat2 = declassify(sortingNetworkSort(mat,i));
		for(uint j = 0; j < 5; ++j){
			if(j != 0){
				if(last > mat2[j,i]){
					result = false;
					break;
				}
				else{
					last = mat2[j,i];
				}
			}
			else{
				last = mat2[j,i];
			}
		}
		if(result){
	 		succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
	 	}
	 	else{
	 		print("FAILURE! sorting process failed");
	 		print("Got this : ");
	 		printMatrix(mat2);
	 		print(" From this: ");
	 		printMatrix(declassify(mat));
	 		print("While sorting column ", column);
	 		all_tests = all_tests +1;
	 		result = true;
	 	}
	}
}

template <domain D : additive3pp, type T>
void sorting_network_matrix2(D T data){
	pd_a3p T[[2]] mat (4,2);
	mat[:,0] = {1,1,0,0};
	mat[:,1] = {1,0,1,0};
	T[[2]] result = declassify(sortingNetworkSort(mat,0::uint,1::uint));
	T[[2]] control (4,2);
	control[:,0] = {0,0,1,1};
	control[:,1] = {0,1,0,1};
 	if(all(result == control)){
		succeeded_tests = succeeded_tests + 1;
		all_tests = all_tests +1;
		print("SUCCESS!");
	}
	else{
		print("FAILURE! sorting process failed");
		print("Got this : ");
		printMatrix(result);
		print(" From this: ");
		printMatrix(declassify(mat));
		all_tests = all_tests +1;
	}
}

template <domain D : additive3pp, type T>
void sorting_network_matrix3(D T data){
	pd_a3p T[[2]] mat (8,3);
	mat[:,0] = {1,1,1,1,0,0,0,0};
	mat[:,1] = {1,1,0,0,1,1,0,0};
	mat[:,2] = {1,0,1,0,1,0,1,0};
	T[[2]] result = declassify(sortingNetworkSort(mat,0::uint,1::uint,2::uint));
	T[[2]] control (8,3);
	control[:,0] = {0,0,0,0,1,1,1,1};
	control[:,1] = {0,0,1,1,0,0,1,1};
	control[:,2] = {0,1,0,1,0,1,0,1};
 	if(all(result == control)){
		succeeded_tests = succeeded_tests + 1;
		all_tests = all_tests +1;
		print("SUCCESS!");
	}
	else{
		print("FAILURE! sorting process failed");
		print("Got this : ");
		printMatrix(result);
		print(" From this: ");
		printMatrix(declassify(mat));
		all_tests = all_tests +1;
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


	print("Sorting test: start");

	print("TEST 1: 1-dimensional 3-8 element vector sorting");
	{
		print("uint8");
		test_sorting(0::uint8);
	}
	{
		print("uint16");
		test_sorting(0::uint16);
	}
	{
		print("uint32");
		test_sorting(0::uint32);
	}
	{
		print("uint64/uint");
		test_sorting(0::uint64);
	}
	{
		print("int8");
		test_sorting(0::int8);
	}
	{
		print("int16");
		test_sorting(0::int16);
	}
	{
		print("int32");
		test_sorting(0::int32);
	}
	{
		print("int64/int");
		test_sorting(0::int64);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		test_sorting_xor(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		test_sorting_xor(data);
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 data = 0;
		test_sorting_xor(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		test_sorting_xor(data);
	}

	// highest bit makes comparison within sorting wrong, to show this i've created TEST 1.b
	print("TEST 1.b: 1-dimensional 6 element vector sorting with highest bit always 0");
	{
		for(uint j = 0; j < 10; ++j){
			print("uint8");
			pd_a3p uint8[[1]] vec (6);
			uint8 check = 0b01111111;
			vec = randomize(vec);
			uint8[[1]] vec2 (6) = declassify(vec);
			for(uint i = 0; i < size(vec);++i){
				while(vec2[i] >= check){
					vec2[i] -= 1;
				}
			}
			vec = vec2;
			test(vec);
		}
		for(uint j = 0; j < 10; ++j){
			print("uint16");
			pd_a3p uint16[[1]] vec (6);
			uint16 check = 0b0111111111111111;
			vec = randomize(vec);
			uint16[[1]] vec2 (6) = declassify(vec);
			for(uint i = 0; i < size(vec);++i){
				while(vec2[i] >= check){
					vec2[i] -= 1;
				}
			}
			vec = vec2;
			test(vec);
		}
	}
	print("TEST 2: 1-dimensional boolean vector sorting");
	{
		{
			pd_a3p bool[[1]] vec (0);
			vec = sort(vec);
		}
		for(uint i = 3; i < 10;++i){
			pd_a3p bool[[1]] vec (i);
			vec = randomize(vec);
			bool result = true;
			bool last;
			bool[[1]] vec2 = declassify(sort(vec));
			for(uint j = 0; j < size(vec);++j){
				if(j != 0){
					if((last == true) && (vec2[j] == false)){
						result = false;
						break;
					}
					else{
						last = vec2[j];
					}
				}
				else{
					last = vec2[j];
				}
			}
			if(result){
		 		succeeded_tests = succeeded_tests + 1;
		 		all_tests = all_tests +1;
		 		print("SUCCESS!");
		 	}
		 	else{
		 		print("FAILURE! sorting process failed");
		 		print("Got this : ",arrayToString(vec2), " From this: ", arrayToString(declassify(vec)));
		 		all_tests = all_tests +1;
		 	}
		}
	}
	print("TEST 3: 2-dimensional (5,5) boolean matrix sorting by all columns");
	{
		{
			pd_a3p bool[[2]] mat(0,2);
			mat = sort(mat,0::uint);
			mat = sort(mat,1::uint);
		}
		pd_a3p bool[[2]] mat (5,5) = false;
		mat = randomize(mat);
		public bool result = true;
		public bool last;
		public uint column;
		for(uint i = 0; i < 5; ++i){
			bool[[2]] mat2 = declassify(sort(mat,i));
			for(uint j = 0; j < 5; ++j){
				if(j != 0){
					if((last == true) && (mat2[j,i] == false)){
						result = false;
						column = i;
						break;
					}
					else{
						last = mat2[j,i];
					}
				}
				else{
					last = mat2[j,i];
				}
			}
			if(result){
		 		succeeded_tests = succeeded_tests + 1;
		 		all_tests = all_tests +1;
		 		print("SUCCESS!");
		 	}
		 	else{
		 		print("FAILURE! sorting process failed");
		 		print("Got this : ");
		 		printMatrix(mat2);
		 		print(" From this: ");
		 		printMatrix(declassify(mat));
		 		print("While sorting column", column);
		 		all_tests = all_tests +1;
		 	}
		}
	}
	print("TEST 4: 2-dimensional (5,5) matrix sorting by all columns");
	{
		print("uint8");
		test_4(0::uint8);
	}
	{
		print("uint16");
		test_4(0::uint16);
	}
	{
		print("uint32");
		test_4(0::uint32);
	}
	{
		print("uint64/uint");
		test_4(0::uint);
	}
	{
		print("int8");
		test_4(0::int8);
	}
	{
		print("int16");
		test_4(0::int16);
	}
	{
		print("int32");
		test_4(0::int32);
	}
	{
		print("int64/int");
		test_4(0::int);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		test_4_xor(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		test_4_xor(data);
	}
	{
		print("uint32");
		pd_a3p xor_uint32 data = 0;
		test_4_xor(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		test_4_xor(data);
	}
	// highest bit makes comparison within sorting wrong.*/
	print("TEST 5: sorting network sort on vectors");
	{
		print("uint8");
		pd_a3p uint8 data = 0;
		sorting_network(data);
	}
	{
		print("uint16");
		pd_a3p uint16 data = 0;
		sorting_network(data);
	}
	{
		print("uint32");
		pd_a3p uint32 data = 0;
		sorting_network(data);
	}
	{
		print("uint64/uint");
		pd_a3p uint data = 0;
		sorting_network(data);
	}
	{
		print("int8");
		pd_a3p int8 data = 0;
		sorting_network(data);
	}
	{
		print("int16");
		pd_a3p int16 data = 0;
		sorting_network(data);
	}
	{
		print("int32");
		pd_a3p int32 data = 0;
		sorting_network(data);
	}
	{
		print("int64/int");
		pd_a3p int data = 0;
		sorting_network(data);
	}
	{ 						
		print("xor_uint8");
		pd_a3p xor_uint8 data = 0;
		sorting_network(data);
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 data = 0;
		sorting_network(data);
	}
	{
		print("uint32");
		pd_a3p xor_uint32 data = 0;
		sorting_network(data);
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 data = 0;
		sorting_network(data);
	}
	print("TEST 6: sorting network sort on matrices");
	{
		print("uint8");
		pd_a3p uint8 data = 0;
		sorting_network_matrix(data);
	}
	{
		print("uint16");
		pd_a3p uint16 data = 0;
		sorting_network_matrix(data);
	}
	{
		print("uint32");
		pd_a3p uint32 data = 0;
		sorting_network_matrix(data);
	}
	{
		print("uint64/uint");
		pd_a3p uint data = 0;
		sorting_network_matrix(data);
	}
	{
		print("int8");
		pd_a3p int8 data = 0;
		sorting_network_matrix(data);
	}
	{
		print("int16");
		pd_a3p int16 data = 0;
		sorting_network_matrix(data);
	}
	{
		print("int32");
		pd_a3p int32 data = 0;
		sorting_network_matrix(data);
	}
	{
		print("int64/int");
		pd_a3p int data = 0;
		sorting_network_matrix(data);
	}
	print("TEST 7: sorting network sort on matrices(2)");
	{
		print("uint8");
		pd_a3p uint8 data = 0;
		sorting_network_matrix2(data);
	}
	{
		print("uint16");
		pd_a3p uint16 data = 0;
		sorting_network_matrix2(data);
	}
	{
		print("uint32");
		pd_a3p uint32 data = 0;
		sorting_network_matrix2(data);
	}
	{
		print("uint64/uint");
		pd_a3p uint data = 0;
		sorting_network_matrix2(data);
	}
	{
		print("int8");
		pd_a3p int8 data = 0;
		sorting_network_matrix2(data);
	}
	{
		print("int16");
		pd_a3p int16 data = 0;
		sorting_network_matrix2(data);
	}
	{
		print("int32");
		pd_a3p int32 data = 0;
		sorting_network_matrix2(data);
	}
	{
		print("int64/int");
		pd_a3p int data = 0;
		sorting_network_matrix2(data);
	}
	print("TEST 8: sorting network sort on matrices(3)");
	{
		print("uint8");
		pd_a3p uint8 data = 0;
		sorting_network_matrix3(data);
	}
	{
		print("uint16");
		pd_a3p uint16 data = 0;
		sorting_network_matrix3(data);
	}
	{
		print("uint32");
		pd_a3p uint32 data = 0;
		sorting_network_matrix3(data);
	}
	{
		print("uint64/uint");
		pd_a3p uint data = 0;
		sorting_network_matrix3(data);
	}
	{
		print("int8");
		pd_a3p int8 data = 0;
		sorting_network_matrix3(data);
	}
	{
		print("int16");
		pd_a3p int16 data = 0;
		sorting_network_matrix3(data);
	}
	{
		print("int32");
		pd_a3p int32 data = 0;
		sorting_network_matrix3(data);
	}
	{
		print("int64/int");
		pd_a3p int data = 0;
		sorting_network_matrix3(data);
	}
	
	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}