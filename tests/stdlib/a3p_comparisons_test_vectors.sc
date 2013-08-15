/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module a3p_comparisons_test_vectors;

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

template<domain D:additive3pp,type T>
void larger_than(D T[[1]] vec, D T[[1]] vec2){
	if(any(declassify(vec2 > vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " > ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
	 	all_tests = all_tests +1;
	 	print("SUCCESS!");
	}
}

template<domain D:additive3pp,type T>
void smaller_than(D T[[1]] vec, D T[[1]] vec2){
	if(any(declassify(vec < vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " < ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
		all_tests = all_tests +1;
		print("SUCCESS!");
	}
}

template<domain D:additive3pp,type T>
void larger_than_equal(D T[[1]] vec, D T[[1]] vec2){
	if(any(declassify(vec2 >= vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " >= ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<domain D:additive3pp,type T>
void smaller_than_equal(D T[[1]] vec, D T[[1]] vec2){
	if(any(declassify(vec <= vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " <= ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
	}
	else{
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
}

template<domain D:additive3pp,type T>
void equal_equal(D T[[1]] vec, D T[[1]] vec2){
	vec = randomize(vec);
	vec2 = vec;
	if(all(declassify(vec == vec2))){
		succeeded_tests = succeeded_tests + 1;
 		all_tests = all_tests +1;
 		print("SUCCESS!");
	}
	else{
		print("FAILURE! ", arrayToString(declassify(vec)) , " == ", arrayToString(declassify(vec2)));
 		all_tests = all_tests +1;
	}
}



void main(){
	print("Comparisons test: start");

	print("TEST 1: > operator");
	{
		print("uint8");
		pd_a3p uint8[[1]] vec (6) = {5,55,105,155,205,255};
		pd_a3p uint8[[1]] vec2 (6) = {4,40,55,2,175,0};
		larger_than(vec,vec2);
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] vec (7) = {5535,15535,25535,35535,45535,55535,65535};
		pd_a3p uint16[[1]] vec2 (7) = {5534, 12546,24,35534,45535,39053,0};
		larger_than(vec,vec2);
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] vec (5) = {67295,1073792295,2147517294,3221242294,4294967294};
		pd_a3p uint32[[1]] vec2 (5) = {67294,21432532,78635892,192468953,0};
		larger_than(vec,vec2);
	}
	{
		print("uint64/uint");
		pd_a3p uint[[1]] vec (4) = {3689348814741910323,7378697629483820646,11068046444225730969,14757395258967641292};
		pd_a3p uint[[1]] vec2 (4) = {3689348814741910322,35597629483820646,1,0};
		larger_than(vec,vec2);
	}
	{
		print("int8");
		pd_a3p int8[[1]] vec (6) = {-127,-77,-27,23,73,123};
		pd_a3p int8[[1]] vec2 (6) = {-128,-78,-27,-1,0,122};
		larger_than(vec,vec2);
	}
	{
		print("int16");
		pd_a3p int16[[1]] vec (8) = {-32767,-24575,-16383,-8191,1,8193,16385,24577};
		pd_a3p int16[[1]] vec2 (8) = {-32768,-25643,-17345,-8191,0,1,2153,21453};
		larger_than(vec,vec2);
	}
	{
		print("int32");
		pd_a3p int32[[1]] vec (8) = {-2147483647,-1610612735,-1073741823,-536870911,1,536870913,1073741825,1610612737};
		pd_a3p int32[[1]] vec2 (8) = {-2147483648,-1610612735,-1243259079,-9537127485,0,-1,1,1610612737};
		larger_than(vec,vec2);
	}
	{
		print("int64/int");
		pd_a3p int[[1]] vec (6) = {-9223372036854775807,-5534023222112865484,-1844674407370955161,1844674407370955162,5534023222112865485,9223372036854775807};
		pd_a3p int[[1]] vec2 (6) = {-9223372036854775808,-8735123222112865484,-6391824407370955161,0,-1,1};
		larger_than(vec,vec2);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8[[1]] vec (6) = {5,55,105,155,205,255};
		pd_a3p xor_uint8[[1]] vec2 (6) = {4,40,55,2,175,0};
		if(any(declassify(vec2 > vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " > ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
		 	all_tests = all_tests +1;
		 	print("SUCCESS!");
		}
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16[[1]] vec (7) = {5535,15535,25535,35535,45535,55535,65535};
		pd_a3p xor_uint16[[1]] vec2 (7) = {5534, 12546,24,35534,45535,39053,0};
		if(any(declassify(vec2 > vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " > ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
		 	all_tests = all_tests +1;
		 	print("SUCCESS!");
		}
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32[[1]] vec (5) = {67295,1073792295,2147517294,3221242294,4294967294};
		pd_a3p xor_uint32[[1]] vec2 (5) = {67294,21432532,78635892,192468953,0};
		if(any(declassify(vec2 > vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " > ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
		 	all_tests = all_tests +1;
		 	print("SUCCESS!");
		}
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64[[1]] vec (4) = {3689348814741910323,7378697629483820646,11068046444225730969,14757395258967641292};
		pd_a3p xor_uint64[[1]] vec2 (4) = {3689348814741910322,35597629483820646,1,0};
		if(any(declassify(vec2 > vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " > ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
		 	all_tests = all_tests +1;
		 	print("SUCCESS!");
		}
	}

	print("TEST 2: < operator");
	{
		print("uint8");
		pd_a3p uint8[[1]] vec (6) = {5,55,105,155,205,255};
		pd_a3p uint8[[1]] vec2 (6) = {4,40,55,2,175,0};
		smaller_than(vec,vec2);
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] vec (7) = {5535,15535,25535,35535,45535,55535,65535};
		pd_a3p uint16[[1]] vec2 (7) = {5534, 12546,24,35534,45535,39053,0};
		smaller_than(vec,vec2);
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] vec (5) = {67295,1073792295,2147517294,3221242294,4294967294};
		pd_a3p uint32[[1]] vec2 (5) = {67294,21432532,78635892,192468953,0};
		smaller_than(vec,vec2);
	}
	{
		print("uint64/uint");
		pd_a3p uint[[1]] vec (4) = {3689348814741910323,7378697629483820646,11068046444225730969,14757395258967641292};
		pd_a3p uint[[1]] vec2 (4) = {3689348814741910322,35597629483820646,1,0};
		smaller_than(vec,vec2);
	}
	{
		print("int8");
		pd_a3p int8[[1]] vec (6) = {-127,-77,-27,23,73,123};
		pd_a3p int8[[1]] vec2 (6) =  {-128,-78,-27,-1,0,122};
		smaller_than(vec,vec2);
	}
	{
		print("int16");
		pd_a3p int16[[1]] vec (8) = {-32767,-24575,-16383,-8191,1,8193,16385,24577};
		pd_a3p int16[[1]] vec2 (8) = {-32768,-25643,-17345,-8191,0,1,2153,21453};
		smaller_than(vec,vec2);
	}
	{
		print("int32");
		pd_a3p int32[[1]] vec (8) = {-2147483647,-1610612735,-1073741823,-536870911,1,536870913,1073741825,1610612737};
		pd_a3p int32[[1]] vec2 (8) = {-2147483648,-1610612735,-1243259079,-9537127485,0,-1,1,1610612737};
		smaller_than(vec,vec2);
	}
	{
		print("int64/int");
		pd_a3p int[[1]] vec (6) = {-9223372036854775807,-5534023222112865484,-1844674407370955161,1844674407370955162,5534023222112865485,9223372036854775807};
		pd_a3p int[[1]] vec2 (6) = {-9223372036854775808,-8735123222112865484,-6391824407370955161,0,-1,1};
		smaller_than(vec,vec2);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8[[1]] vec (6) = {5,55,105,155,205,255};
		pd_a3p xor_uint8[[1]] vec2 (6) = {4,40,55,2,175,0};
		if(any(declassify(vec < vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " < ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16[[1]] vec (7) = {5535,15535,25535,35535,45535,55535,65535};
		pd_a3p xor_uint16[[1]] vec2 (7) = {5534, 12546,24,35534,45535,39053,0};
		if(any(declassify(vec < vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " < ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32[[1]] vec (5) = {67295,1073792295,2147517294,3221242294,4294967294};
		pd_a3p xor_uint32[[1]] vec2 (5) = {67294,21432532,78635892,192468953,0};
		if(any(declassify(vec < vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " < ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64[[1]] vec (4) = {3689348814741910323,7378697629483820646,11068046444225730969,14757395258967641292};
		pd_a3p xor_uint64[[1]] vec2 (4) = {3689348814741910322,35597629483820646,1,0};
		if(any(declassify(vec < vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " < ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}

	print("TEST 3: >= operator");
	{
		print("uint8");
		pd_a3p uint8[[1]] vec (6) = {5,55,105,155,205,255};
		pd_a3p uint8[[1]] vec2 (6) = {4,40,55,2,175,0};
		larger_than_equal(vec,vec2);
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] vec (7) = {5535,15535,25535,35535,45535,55535,65535};
		pd_a3p uint16[[1]] vec2 (7) = {5534, 12546,24,35534,45534,39053,0};
		larger_than_equal(vec,vec2);
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] vec (5) = {67295,1073792295,2147517294,3221242294,4294967294};
		pd_a3p uint32[[1]] vec2 (5) = {67294,21432532,78635892,192468953,0};
		larger_than_equal(vec,vec2);
	}
	{
		print("uint64/uint");
		pd_a3p uint[[1]] vec (4) = {3689348814741910323,7378697629483820646,11068046444225730969,14757395258967641292};
		pd_a3p uint[[1]] vec2 (4) ={3689348814741910322,35597629483820646,1,0};
		larger_than_equal(vec,vec2);
	}
	{
		print("int8");
		pd_a3p int8[[1]] vec (6) = {-127,-77,-27,23,73,123};
		pd_a3p int8[[1]] vec2 (6) = {-128,-78,-28,-1,0,122};
		larger_than_equal(vec,vec2);
	}
	{
		print("int16");
		pd_a3p int16[[1]] vec (8) = {-32767,-24575,-16383,-8191,1,8193,16385,24577};
		pd_a3p int16[[1]] vec2 (8) = {-32768,-25643,-17345,-8192,0,1,2153,21453};
		larger_than_equal(vec,vec2);
	}
	{
		print("int32");
		pd_a3p int32[[1]] vec (8) = {-2147483647,-1610612735,-1073741823,-536870911,1,536870913,1073741825,1610612737};
		pd_a3p int32[[1]] vec2 (8) = {-2147483648,-1610612736,-1243259079,-9537127485,0,-1,1,1610612736};
		larger_than_equal(vec,vec2);
	}
	{
		print("int64/int");
		pd_a3p int[[1]] vec (6) = {-9223372036854775807,-5534023222112865484,-1844674407370955161,1844674407370955162,5534023222112865485,9223372036854775807};
		pd_a3p int[[1]] vec2 (6) = {-9223372036854775808,-8735123222112865484,-6391824407370955161,0,-1,1};
		larger_than_equal(vec,vec2);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8[[1]] vec (6) = {5,55,105,155,205,255};
		pd_a3p xor_uint8[[1]] vec2 (6) = {4,40,55,2,175,0};
		if(any(declassify(vec2 >= vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " >= ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16[[1]] vec (7) = {5535,15535,25535,35535,45535,55535,65535};
		pd_a3p xor_uint16[[1]] vec2 (7) = {5534, 12546,24,35534,45534,39053,0};
		if(any(declassify(vec2 >= vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " >= ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32[[1]] vec (5) = {67295,1073792295,2147517294,3221242294,4294967294};
		pd_a3p xor_uint32[[1]] vec2 (5) = {67294,21432532,78635892,192468953,0};
		if(any(declassify(vec2 >= vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " >= ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64[[1]] vec (4) = {3689348814741910323,7378697629483820646,11068046444225730969,14757395258967641292};
		pd_a3p xor_uint64[[1]] vec2 (4) = {3689348814741910322,35597629483820646,1,0};
		if(any(declassify(vec2 >= vec))){
			print("FAILURE! ", arrayToString(declassify(vec2)) , " >= ", arrayToString(declassify(vec)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	print("TEST 4: <= operator");
	{
		print("uint8");
		pd_a3p uint8[[1]] vec (6) = {5,55,105,155,205,255};
		pd_a3p uint8[[1]] vec2 (6) = {4,40,55,2,175,0};
		smaller_than_equal(vec,vec2);
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] vec (7) = {5535,15535,25535,35535,45535,55535,65535};
		pd_a3p uint16[[1]] vec2 (7) = {5534, 12546,24,35534,45534,39053,0};
		smaller_than_equal(vec,vec2);
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] vec (5) = {67295,1073792295,2147517294,3221242294,4294967294};
		pd_a3p uint32[[1]] vec2 (5) = {67294,21432532,78635892,192468953,0};
		smaller_than_equal(vec,vec2);
	}
	{
		print("uint64/uint");
		pd_a3p uint[[1]] vec (4) = {3689348814741910323,7378697629483820646,11068046444225730969,14757395258967641292};
		pd_a3p uint[[1]] vec2 (4) = {3689348814741910322,35597629483820646,1,0};
		smaller_than_equal(vec,vec2);
	}
	{
		print("int8");
		pd_a3p int8[[1]] vec (6) = {-127,-77,-27,23,73,123};
		pd_a3p int8[[1]] vec2 (6) = {-128,-78,-28,-1,0,122};
		smaller_than_equal(vec,vec2);
	}
	{
		print("int16");
		pd_a3p int16[[1]] vec (8) = {-32767,-24575,-16383,-8191,1,8193,16385,24577};
		pd_a3p int16[[1]] vec2 (8) = {-32768,-25643,-17345,-8192,0,1,2153,21453};
		smaller_than_equal(vec,vec2);
	}
	{
		print("int32");
		pd_a3p int32[[1]] vec (8) = {-2147483647,-1610612735,-1073741823,-536870911,1,536870913,1073741825,1610612737};
		pd_a3p int32[[1]] vec2 (8) = {-2147483648,-1610612736,-1243259079,-9537127485,0,-1,1,1610612736};
		smaller_than_equal(vec,vec2);
	}
	{
		print("int64/int");
		pd_a3p int[[1]] vec (6) = {-9223372036854775807,-5534023222112865484,-1844674407370955161,1844674407370955162,5534023222112865485,9223372036854775807};
		pd_a3p int[[1]] vec2 (6) =  {-9223372036854775808,-8735123222112865484,-6391824407370955161,0,-1,1};
		smaller_than_equal(vec,vec2);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8[[1]] vec (6) = {5,55,105,155,205,255};
		pd_a3p xor_uint8[[1]] vec2 (6) = {4,40,55,2,175,0};
		if(any(declassify(vec <= vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " <= ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16[[1]] vec (7) = {5535,15535,25535,35535,45535,55535,65535};
		pd_a3p xor_uint16[[1]] vec2 (7) = {5534, 12546,24,35534,45534,39053,0};
		if(any(declassify(vec <= vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " <= ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32[[1]] vec (5) = {67295,1073792295,2147517294,3221242294,4294967294};
		pd_a3p xor_uint32[[1]] vec2 (5) = {67294,21432532,78635892,192468953,0};
		if(any(declassify(vec <= vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " <= ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64[[1]] vec (4) = {3689348814741910323,7378697629483820646,11068046444225730969,14757395258967641292};
		pd_a3p xor_uint64[[1]] vec2 (4) = {3689348814741910322,35597629483820646,1,0};
		if(any(declassify(vec <= vec2))){
			print("FAILURE! ", arrayToString(declassify(vec)) , " <= ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
		else{
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
	}

	print("TEST 5: == operator");
	{
		print("bool");
		pd_a3p bool[[1]] vec (6);
		pd_a3p bool[[1]] vec2 (6);
		equal_equal(vec,vec2);
	}
	{
		print("uint8");
		pd_a3p uint8[[1]] vec (6);
		pd_a3p uint8[[1]] vec2 (6);
		equal_equal(vec,vec2);
	}
	{
		print("uint16");
		pd_a3p uint16[[1]] vec (6);
		pd_a3p uint16[[1]] vec2 (6);
		equal_equal(vec,vec2);
	}
	{
		print("uint32");
		pd_a3p uint32[[1]] vec (6);
		pd_a3p uint32[[1]] vec2 (6);
		equal_equal(vec,vec2);
	}
	{
		print("uint64/uint");
		pd_a3p uint[[1]] vec (6);
		pd_a3p uint[[1]] vec2 (6);
		equal_equal(vec,vec2);
	}
	{
		print("int8");
		pd_a3p int8[[1]] vec (6);
		pd_a3p int8[[1]] vec2 (6);
		equal_equal(vec,vec2);
	}
	{
		print("int16");
		pd_a3p int16[[1]] vec (6);
		pd_a3p int16[[1]] vec2 (6);
		equal_equal(vec,vec2);
	}
	{
		print("int32");
		pd_a3p int32[[1]] vec (6);
		pd_a3p int32[[1]] vec2 (6);
		equal_equal(vec,vec2);
	}
	{
		print("int64/int");
		pd_a3p int[[1]] vec (6);
		pd_a3p int[[1]] vec2 (6);
		equal_equal(vec,vec2);
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8[[1]] vec (6);
		pd_a3p xor_uint8[[1]] vec2 (6);
		vec = randomize(vec);
		vec2 = vec;
		if(all(declassify(vec == vec2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", arrayToString(declassify(vec)) , " == ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16[[1]] vec (6);
		pd_a3p xor_uint16[[1]] vec2 (6);
		vec = randomize(vec);
		vec2 = vec;
		if(all(declassify(vec == vec2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", arrayToString(declassify(vec)) , " == ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32[[1]] vec (6);
		pd_a3p xor_uint32[[1]] vec2 (6);
		vec = randomize(vec);
		vec2 = vec;
		if(all(declassify(vec == vec2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", arrayToString(declassify(vec)) , " == ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64[[1]] vec (6);
		pd_a3p xor_uint64[[1]] vec2 (6);
		vec = randomize(vec);
		vec2 = vec;
		if(all(declassify(vec == vec2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", arrayToString(declassify(vec)) , " == ", arrayToString(declassify(vec2)));
	 		all_tests = all_tests +1;
		}
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}