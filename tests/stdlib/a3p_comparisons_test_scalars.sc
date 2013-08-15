/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module a3p_comparisons_test_scalars;

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

void main(){
	print("Comparisons test: start");

	print("TEST 1: > operator");
 	{
 		print("uint8");
 		pd_a3p uint8 scalar = 4;
 		pd_a3p uint8 scalar2;
 		bool result = true;
 		for(uint i = 4 + 1; i < 255; i = i + 50){
 			scalar2 = (uint8)i;
 			if(declassify(scalar > scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint16");
 		pd_a3p uint16 scalar = 5534;
 		pd_a3p uint16 scalar2;
 		bool result = true;
 		for(uint i = 5534 + 1; i < 65535; i = i + 10000){
 			scalar2 = (uint16)i;
 			if(declassify(scalar > scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint32");
 		pd_a3p uint32 scalar = 67294;
 		pd_a3p uint32 scalar2;
 		bool result = true;
 		for(uint i = 67294 + 1; i < 4294967295; i = i + 1073725000){
 			scalar2 = (uint32)i;
 			if(declassify(scalar > scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint64/uint");
 		pd_a3p uint scalar = 3689348814741910322;
 		pd_a3p uint scalar2;
 		bool result = true;
 		for(uint i = 3689348814741910322 + 1; i < 18446744073709551615; i = i + 3689348814741910323){
 			scalar2 = i;
 			if(declassify(scalar > scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int8");
 		pd_a3p int8 scalar = -128;
 		pd_a3p int8 scalar2;
 		bool result = true;
 		for(int i = -128 + 1; i < 127; i = i + 50){
 			scalar2 = (int8)i;
 			if(declassify(scalar > scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int16");
 		pd_a3p int16 scalar = -32768;
 		pd_a3p int16 scalar2;
 		bool result = true;
 		for(int i = -32768 + 1; i < 32767; i = i + 8192){
 			scalar2 = (int16)i;
 			if(declassify(scalar > scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint32");
 		pd_a3p int32 scalar = -2147483648;
 		pd_a3p int32 scalar2;
 		bool result = true;
 		for(int i = -2147483648 + 1; i < 2147483647; i = i + 536870912){
 			scalar2 = (int32)i;
 			if(declassify(scalar > scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int64/int");
 		pd_a3p int scalar = -9223372036854775808;
 		pd_a3p int scalar2;
 		bool result = true;
 		for(int i = -9223372036854775808 + 1; i < 5534023222112865000; i = i + 3689348814741910323){
 			scalar2 = i;
 			if(declassify(scalar > scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
		print("xor_uint8");
		bool result = true;
		pd_a3p xor_uint8 scalar = 4;
		pd_a3p xor_uint8 scalar2 = 5;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 55;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 105;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 155;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 205;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 255;
		if(declassify(scalar > scalar2)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
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
		bool result = true;
		pd_a3p xor_uint16 scalar = 5534;
		pd_a3p xor_uint16 scalar2 = 5535;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 15535;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 25535;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 35535;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 45535;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 55535;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 65535;
		if(declassify(scalar > scalar2)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
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
		bool result = true;
		pd_a3p xor_uint32 scalar = 67294;
		pd_a3p xor_uint32 scalar2 = 67295;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 1073792295;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 2147517294;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 3221242294;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 4294967294;
		if(declassify(scalar > scalar2)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
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
		bool result = true;
		pd_a3p xor_uint64 scalar = 3689348814741910322;
		pd_a3p xor_uint64 scalar2 = 3689348814741910323;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 7378697629483820646;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 11068046444225730969;
		if(declassify(scalar > scalar2)){
			result = false;
		}
		scalar2 = 14757395258967641292;
		if(declassify(scalar > scalar2)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar) , " > ", declassify(scalar2));
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
 		pd_a3p uint8 scalar = 4;
 		pd_a3p uint8 scalar2;
 		bool result = true;
 		for(uint i = 4 + 1; i < 255; i = i + 50){
 			scalar2 = (uint8)i;
 			if(declassify(scalar2 < scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint16");
 		pd_a3p uint16 scalar = 5534;
 		pd_a3p uint16 scalar2;
 		bool result = true;
 		for(uint i = 5534 + 1; i < 65535; i = i + 10000){
 			scalar2 = (uint16)i;
 			if(declassify(scalar2 < scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint32");
 		pd_a3p uint32 scalar = 67294;
 		pd_a3p uint32 scalar2;
 		bool result = true;
 		for(uint i = 67294 + 1; i < 4294967295; i = i + 1073725000){
 			scalar2 = (uint32)i;
 			if(declassify(scalar2 < scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint64/uint");
 		pd_a3p uint scalar = 3689348814741910322;
 		pd_a3p uint scalar2;
 		bool result = true;
 		for(uint i = 3689348814741910322 + 1; i < 18446744073709551615; i = i + 3689348814741910323){
 			scalar2 = i;
 			if(declassify(scalar2 < scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int8");
 		pd_a3p int8 scalar = -128;
 		pd_a3p int8 scalar2;
 		bool result = true;
 		for(int i = -128 + 1; i < 127; i = i + 50){
 			scalar2 = (int8)i;
 			if(declassify(scalar2 < scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int16");
 		pd_a3p int16 scalar = -32768;
 		pd_a3p int16 scalar2;
 		bool result = true;
 		for(int i = -32768 + 1; i < 32767; i = i + 8192){
 			scalar2 = (int16)i;
 			if(declassify(scalar2 < scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int32");
 		pd_a3p int32 scalar = -2147483648;
 		pd_a3p int32 scalar2;
 		bool result = true;
 		for(int i = -2147483648 + 1; i < 2147483647; i = i + 536870912){
 			scalar2 = (int32)i;
 			if(declassify(scalar2 < scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int64/int");
 		pd_a3p int scalar = -9223372036854775808;
 		pd_a3p int scalar2;
 		bool result = true;
 		for(int i = -9223372036854775808 + 1; i < 5534023222112865000; i = i + 3689348814741910323){
 			scalar2 = i;
 			if(declassify(scalar2 < scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
		print("xor_uint8");
		bool result = true;
		pd_a3p xor_uint8 scalar = 4;
		pd_a3p xor_uint8 scalar2 = 5;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 55;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 105;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 155;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 205;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 255;
		if(declassify(scalar2 < scalar)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
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
		bool result = true;
		pd_a3p xor_uint16 scalar = 5534;
		pd_a3p xor_uint16 scalar2 = 5535;
		if(declassify(scalar2 < scalar2)){
			result = false;
		}
		scalar2 = 15535;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 25535;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 35535;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 45535;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 55535;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 65535;
		if(declassify(scalar2 < scalar)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
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
		bool result = true;
		pd_a3p xor_uint32 scalar = 67294;
		pd_a3p xor_uint32 scalar2 = 67295;
		if(declassify(scalar2 < scalar2)){
			result = false;
		}
		scalar2 = 1073792295;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 2147517294;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 3221242294;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 4294967294;
		if(declassify(scalar2 < scalar)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
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
		bool result = true;
		pd_a3p xor_uint64 scalar = 3689348814741910322;
		pd_a3p xor_uint64 scalar2 = 3689348814741910323;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 7378697629483820646;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 11068046444225730969;
		if(declassify(scalar2 < scalar)){
			result = false;
		}
		scalar2 = 14757395258967641292;
		if(declassify(scalar2 < scalar)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar2) , " < ", declassify(scalar));
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
 		pd_a3p uint8 scalar = 4;
 		pd_a3p uint8 scalar2;
 		bool result = true;
 		for(uint i = 4 + 1; i < 255; i = i + 50){
 			scalar2 = (uint8)i;
 			if(declassify(scalar >= scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint16");
 		pd_a3p uint16 scalar = 5534;
 		pd_a3p uint16 scalar2;
 		bool result = true;
 		for(uint i = 5534 + 1; i < 65535; i = i + 10000){
 			scalar2 = (uint16)i;
 			if(declassify(scalar >= scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint32");
 		pd_a3p uint32 scalar = 67294;
 		pd_a3p uint32 scalar2;
 		bool result = true;
 		for(uint i = 67294 + 1; i < 4294967295; i = i + 1073725000){
 			scalar2 = (uint32)i;
 			if(declassify(scalar >= scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint64/uint");
 		pd_a3p uint scalar = 3689348814741910322;
 		pd_a3p uint scalar2;
 		bool result = true;
 		for(uint i = 3689348814741910322 + 1; i < 18446744073709551615; i = i + 3689348814741910323){
 			scalar2 = i;
 			if(declassify(scalar >= scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int8");
 		pd_a3p int8 scalar = -128;
 		pd_a3p int8 scalar2;
 		bool result = true;
 		for(int i = -128 + 1; i < 127; i = i + 50){
 			scalar2 = (int8)i;
 			if(declassify(scalar >= scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int16");
 		pd_a3p int16 scalar = -32768;
 		pd_a3p int16 scalar2;
 		bool result = true;
 		for(int i = -32768 + 1; i < 32767; i = i + 8192){
 			scalar2 = (int16)i;
 			if(declassify(scalar >= scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int32");
 		pd_a3p int32 scalar = -2147483648;
 		pd_a3p int32 scalar2;
 		bool result = true;
 		for(int i = -2147483648 + 1; i < 2147483647; i = i + 536870912){
 			scalar2 = (int32)i;
 			if(declassify(scalar >= scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int64/int");
 		pd_a3p int scalar = -9223372036854775808;
 		pd_a3p int scalar2;
 		bool result = true;
 		for(int i = -9223372036854775808 + 1; i < 5534023222112865000; i = i + 3689348814741910323){
 			scalar2 = i;
 			if(declassify(scalar >= scalar2)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
{
		print("xor_uint8");
		bool result = true;
		pd_a3p xor_uint8 scalar = 4;
		pd_a3p xor_uint8 scalar2 = 5;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 55;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 105;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 155;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 205;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 255;
		if(declassify(scalar >= scalar2)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
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
		bool result = true;
		pd_a3p xor_uint16 scalar = 5534;
		pd_a3p xor_uint16 scalar2 = 5535;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 15535;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 25535;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 35535;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 45535;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 55535;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 65535;
		if(declassify(scalar >= scalar2)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
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
		bool result = true;
		pd_a3p xor_uint32 scalar = 67294;
		pd_a3p xor_uint32 scalar2 = 67295;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 1073792295;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 2147517294;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 3221242294;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 4294967294;
		if(declassify(scalar >= scalar2)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
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
		bool result = true;
		pd_a3p xor_uint64 scalar = 3689348814741910322;
		pd_a3p xor_uint64 scalar2 = 3689348814741910323;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 7378697629483820646;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 11068046444225730969;
		if(declassify(scalar >= scalar2)){
			result = false;
		}
		scalar2 = 14757395258967641292;
		if(declassify(scalar >= scalar2)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar) , " >= ", declassify(scalar2));
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
 		pd_a3p uint8 scalar = 4;
 		pd_a3p uint8 scalar2;
 		bool result = true;
 		for(uint i = 4 + 1; i < 255; i = i + 50){
 			scalar2 = (uint8)i;
 			if(declassify(scalar2 <= scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint16");
 		pd_a3p uint16 scalar = 5534;
 		pd_a3p uint16 scalar2;
 		bool result = true;
 		for(uint i = 5534 + 1; i < 65535; i = i + 10000){
 			scalar2 = (uint16)i;
 			if(declassify(scalar2 <= scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint32");
 		pd_a3p uint32 scalar = 67294;
 		pd_a3p uint32 scalar2;
 		bool result = true;
 		for(uint i = 67294 + 1; i < 4294967295; i = i + 1073725000){
 			scalar2 = (uint32)i;
 			if(declassify(scalar2 <= scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("uint64/uint");
 		pd_a3p uint scalar = 3689348814741910322;
 		pd_a3p uint scalar2;
 		bool result = true;
 		for(uint i = 3689348814741910322 + 1; i < 18446744073709551615; i = i + 3689348814741910323){
 			scalar2 = i;
 			if(declassify(scalar2 <= scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int8");
 		pd_a3p int8 scalar = -128;
 		pd_a3p int8 scalar2;
 		bool result = true;
 		for(int i = -128 + 1; i < 127; i = i + 50){
 			scalar2 = (int8)i;
 			if(declassify(scalar2 <= scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int16");
 		pd_a3p int16 scalar = -32768;
 		pd_a3p int16 scalar2;
 		bool result = true;
 		for(int i = -32768 + 1; i < 32767; i = i + 8192){
 			scalar2 = (int16)i;
 			if(declassify(scalar2 <= scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int32");
 		pd_a3p int32 scalar = -2147483648;
 		pd_a3p int32 scalar2;
 		bool result = true;
 		for(int i = -2147483648 + 1; i < 2147483647; i = i + 536870912){
 			scalar2 = (int32)i;
 			if(declassify(scalar2 <= scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
 		print("int64/int");
 		pd_a3p int scalar = -9223372036854775808;
 		pd_a3p int scalar2;
 		bool result = true;
 		for(int i = -9223372036854775808 + 1; i < 5534023222112865000; i = i + 3689348814741910323){
 			scalar2 = i;
 			if(declassify(scalar2 <= scalar)){
 				result = false;
 			}
 			if(!result){
 				print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
	 			all_tests = all_tests +1;
 				break;
 			}
 		}
 		if(result){
 			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
 		}
	}
	{
		print("xor_uint8");
		bool result = true;
		pd_a3p xor_uint8 scalar = 4;
		pd_a3p xor_uint8 scalar2 = 5;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 55;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 105;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 155;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 205;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 255;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
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
		bool result = true;
		pd_a3p xor_uint16 scalar = 5534;
		pd_a3p xor_uint16 scalar2 = 5535;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 15535;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 25535;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 35535;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 45535;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 55535;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 65535;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
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
		bool result = true;
		pd_a3p xor_uint32 scalar = 67294;
		pd_a3p xor_uint32 scalar2 = 67295;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 1073792295;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 2147517294;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 3221242294;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 4294967294;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
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
		bool result = true;
		pd_a3p xor_uint64 scalar = 3689348814741910322;
		pd_a3p xor_uint64 scalar2 = 3689348814741910323;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 7378697629483820646;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 11068046444225730969;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}
		scalar2 = 14757395258967641292;
		if(declassify(scalar2 <= scalar)){
			result = false;
		}

		if(!result){
			print("FAILURE! ", declassify(scalar2) , " <= ", declassify(scalar));
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
		pd_a3p bool scalar = true;
		pd_a3p bool scalar2 = true;
		if(declassify(scalar == scalar2)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar2) , " == ", declassify(scalar));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("uint8");
		bool result = true;
		pd_a3p uint8 scalar;
		pd_a3p uint8 scalar2;
		for(uint i = 0; i < 10; ++i){
			scalar = randomize(scalar);
			scalar2 = scalar;
			if(!declassify(scalar == scalar2)){
				result = false;
				break;
			}
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar2) , " == ", declassify(scalar));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("uint16");
		bool result = true;
		pd_a3p uint16 scalar;
		pd_a3p uint16 scalar2;
		for(uint i = 0; i < 10; ++i){
			scalar = randomize(scalar);
			scalar2 = scalar;
			if(!declassify(scalar == scalar2)){
				result = false;
				break;
			}
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar2) , " == ", declassify(scalar));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("uint32");
		bool result = true;
		pd_a3p uint32 scalar;
		pd_a3p uint32 scalar2;
		for(uint i = 0; i < 10; ++i){
			scalar = randomize(scalar);
			scalar2 = scalar;
			if(!declassify(scalar == scalar2)){
				result = false;
			}
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar2) , " == ", declassify(scalar));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("uint64/uint");
		bool result = true;
		pd_a3p uint scalar;
		pd_a3p uint scalar2;
		for(uint i = 0; i < 10; ++i){
			scalar = randomize(scalar);
			scalar2 = scalar;
			if(!declassify(scalar == scalar2)){
				result = false;
			}
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar2) , " == ", declassify(scalar));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("int8");
		bool result = true;
		pd_a3p int8 scalar;
		pd_a3p int8 scalar2;
		for(uint i = 0; i < 10; ++i){
			scalar = randomize(scalar);
			scalar2 = scalar;
			if(!declassify(scalar == scalar2)){
				result = false;
				break;
			}
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar2) , " == ", declassify(scalar));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("int16");
		bool result = true;
		pd_a3p int16 scalar;
		pd_a3p int16 scalar2;
		for(uint i = 0; i < 10; ++i){
			scalar = randomize(scalar);
			scalar2 = scalar;
			if(!declassify(scalar == scalar2)){
				result = false;
				break;
			}
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar2) , " == ", declassify(scalar));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("int32");
		bool result = true;
		pd_a3p int32 scalar;
		pd_a3p int32 scalar2;
		for(uint i = 0; i < 10; ++i){
			scalar = randomize(scalar);
			scalar2 = scalar;
			if(!declassify(scalar == scalar2)){
				result = false;
			}
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar2) , " == ", declassify(scalar));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("int64/int");
		bool result = true;
		pd_a3p int scalar;
		pd_a3p int scalar2;
		for(uint i = 0; i < 10; ++i){
			scalar = randomize(scalar);
			scalar2 = scalar;
			if(!declassify(scalar == scalar2)){
				result = false;
			}
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar2) , " == ", declassify(scalar));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("xor_uint8");
		pd_a3p xor_uint8 scalar;
		pd_a3p xor_uint8 scalar2;
		scalar = randomize(scalar);
		scalar2 = scalar;
		if(all(declassify(scalar == scalar2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar) , " == ", declassify(scalar2));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("xor_uint16");
		pd_a3p xor_uint16 scalar;
		pd_a3p xor_uint16 scalar2;
		scalar = randomize(scalar);
		scalar2 = scalar;
		if(all(declassify(scalar == scalar2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar) , " == ", declassify(scalar2));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("xor_uint32");
		pd_a3p xor_uint32 scalar;
		pd_a3p xor_uint32 scalar2;
		scalar = randomize(scalar);
		scalar2 = scalar;
		if(all(declassify(scalar == scalar2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar) , " == ", declassify(scalar2));
	 		all_tests = all_tests +1;
		}
	}
	{
		print("xor_uint64");
		pd_a3p xor_uint64 scalar;
		pd_a3p xor_uint64 scalar2;
		scalar = randomize(scalar);
		scalar2 = scalar;
		if(all(declassify(scalar == scalar2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! ", declassify(scalar) , " == ", declassify(scalar2));
	 		all_tests = all_tests +1;
		}
	}




	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}