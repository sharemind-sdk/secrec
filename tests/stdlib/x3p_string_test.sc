/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module x3p_string_test;

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

	print("TEST 1: string to xor_uint8 vector and back to string");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			string result = bl_strDeclassify(vec);
		}
		string test = "The quick brown fox jumps over the lazy dog";
		pd_a3p xor_uint8[[1]] control (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p xor_uint8[[1]] str = bl_str(test);

		if(all(declassify(str == control))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected this: ", arrayToString(declassify(control)) , " got this: ", arrayToString(declassify(str)));
	 		all_tests = all_tests +1;
		}

		string test2 = bl_strDeclassify(str);
		if(test == test2){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected this: ", test , " got this: ", test2);
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 2: CRC16/32 hash generation with initial hash 0");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			pd_a3p xor_uint32 hash = CRC32(str);
			pd_a3p xor_uint16 hash2 = CRC16(str);
		}
		string test = "The quick brown fox jumps over the lazy dog";
		pd_a3p xor_uint8[[1]] str = bl_str(test);

		pd_a3p xor_uint32 hash = CRC32(str);
		pd_a3p xor_uint32 control = 1095738169;

		if(declassify(hash == control)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected this: ", declassify(control) , " got this: ", declassify(hash));
	 		all_tests = all_tests +1;
		}
		pd_a3p xor_uint16 hash2 = CRC16(str);
		pd_a3p xor_uint16 control2 = 64735;

		if(declassify(hash == control)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected this: ", declassify(control) , " got this: ", declassify(hash));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 3: Count zeroes function");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			pd_a3p uint result = countZeroes(str);
		}
		pd_a3p xor_uint8[[1]] str (15) = 0;
		if(declassify(countZeroes(str)) == 15){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected: 15 got: ", declassify(countZeroes(str)));
	 		all_tests = all_tests +1;
		}
		str = randomize(str);
		for(uint i = 0; i < 15; i = i + 2){
			str[i] = 0;
		}
		if(declassify(countZeroes(str)) == 8){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected: 8 got: ", declassify(countZeroes(str)));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 4: is string empty function");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			pd_a3p bool result = bl_strIsEmpty(str);
		}
		pd_a3p xor_uint8[[1]] str (15) = 0;
		if(declassify(bl_strIsEmpty(str))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected: true got: ", declassify(bl_strIsEmpty(str)));
	 		all_tests = all_tests +1;
		}
		str = randomize(str);
		for(uint i = 0; i < 15; i = i + 2){
			str[i] = 0;
		}
		if(!declassify(bl_strIsEmpty(str))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected: false got: ", declassify(bl_strIsEmpty(str)));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 5: String length function");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			pd_a3p uint length = bl_strLength(str);
		}
		pd_a3p xor_uint8[[1]] str (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p uint length = bl_strLength(str);
		if(declassify(length) == 43){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected: 43 got: ", declassify(length));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 6: String trimming function");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			pd_a3p xor_uint8[[1]] length = bl_strTrim(str);
		}
		pd_a3p xor_uint8[[1]] str (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,0,0,0,0,0,0,0,0,0,0,0,0,0};

		pd_a3p uint length = bl_strLength(bl_strTrim(str));
		if(declassify(length) == 30){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected length after trimming: 30 got: ", declassify(length));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 7: 2 string are equal function");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			pd_a3p xor_uint8[[1]] str2 (0);
			pd_a3p bool result = bl_strEquals(str,str2);
		}
		pd_a3p xor_uint8[[1]] str (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p xor_uint8[[1]] str2 (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p xor_uint8[[1]] str3 (50) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103,0,0,0,0,0,0,0};
		if(declassify(bl_strEquals(str,str2)) && declassify(bl_strEquals(str,str3)) && declassify(bl_strEquals(str3,str2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! The three test strings are not equal");
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 8: Sorting permutation function");
	{
		{
			pd_a3p bool[[1]] arr (0);
			pd_a3p uint[[1]] result = findSortingPermutation(arr);
		}
		pd_a3p bool[[1]] temp (6);
		temp = randomize(temp);
		pd_a3p uint[[1]] temp2 = findSortingPermutation(temp);
		uint[[1]] permutation = declassify(temp2);
		bool[[1]] vec = declassify(temp);
		bool[[1]] vec2 (6);
		for(uint i = 0; i < 6;++i){
			for(uint j = 0; j < 6; ++j){
				if(permutation[j] == i){
					vec2[i] = vec[j];
					break;
				}
			}
		} 
		bool result = true;
		bool last;
		for(uint i = 0; i < 6; ++i){
			if(i == 0){
				last = vec2[i];
			}
			else{
				if(last == false && vec2[i] == true){
					result = false;
					break;
				}
				else{
					last = vec2[i];
				}
			}
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! sorting permutation: ",arrayToString(permutation), " for array ", arrayToString(vec), " is wrong");
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 9: String concatenation");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			pd_a3p xor_uint8[[1]] str2 (0);
			pd_a3p xor_uint8[[1]] str3 = bl_strCat(str,str2);
		}
		pd_a3p xor_uint8[[1]] str (21) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106};
		pd_a3p xor_uint8[[1]] str2 (22) = {117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p xor_uint8[[1]] control (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p xor_uint8[[1]] str3 = bl_strCat(str,str2);
		if(all(declassify(str3 == control))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Concatenating strings returned this: ", arrayToString(declassify(str3)), " but expected this: ", arrayToString(declassify(control)));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 10: String zero extend function");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			pd_a3p xor_uint8[[1]] result = zeroExtend(str,5::uint);
		}
		pd_a3p xor_uint8[[1]] str (3) =  {84,104,101};
		pd_a3p xor_uint8[[1]] str2 = zeroExtend(str,8::uint);
		pd_a3p xor_uint8[[1]] str3 = {84,104,101,0,0,0,0,0};
		if(declassify(bl_strEquals(str,str2)) && declassify(countZeroes(str2) == 5)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Zero extending string returned this: ", arrayToString(declassify(str2)), " but expected this: ", arrayToString(declassify(str3)));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 11: string alphabetizing function");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p bool result = bl_strIsLessThan(vec,vec2);
		}
		string str = "This is a test";
		string str2 = "This is also a test";
		pd_a3p xor_uint8[[1]] vec = bl_str(str);
		pd_a3p xor_uint8[[1]] vec2 = bl_str(str2);
		if(declassify(bl_strIsLessThan(vec,vec2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Alphabetizing failed. Expected ", str ," < ", str2);
	 		all_tests = all_tests +1;
		}
		str = "aaaaabbbccdd";
		str2 = "aaaaaaaaaaaa";
		vec = bl_str(str);
		vec2 = bl_str(str2);
		if(declassify(bl_strIsLessThan(vec2,vec))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Alphabetizing failed. Expected ", str2 ," < ", str);
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 12: Levenshtein distance");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p uint distance = bl_strLevenshtein(vec,vec2);
		}
		string str = "this is a test";
		string str2 = "these are two tests";
		pd_a3p xor_uint8[[1]] vec = bl_str(str);
		pd_a3p xor_uint8[[1]] vec2 = bl_str(str2);
		pd_a3p uint distance = bl_strLevenshtein(vec,vec2);
		if(declassify(distance) == 9){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Levenshtein distance failed. Expected 9 but got: " ,declassify(distance));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 13: String contains function");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p bool result = bl_strContains(vec,vec2);
		}
		string str = "The quick brown fox jumps over the lazy dog";
		pd_a3p xor_uint8[[1]] vec = bl_str(str);
		string sub_str = "k brown f";
		pd_a3p xor_uint8[[1]] vec2 = bl_str(sub_str);
		string sub_str2 = "mps over th";
		pd_a3p xor_uint8[[1]] vec3 = bl_str(sub_str2);
		if(declassify(bl_strContains(vec,vec2)) && declassify(bl_strContains(vec,vec3))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! String contains function returned that: ", str, " does not contain: ", sub_str, " or ", sub_str2);
	 		all_tests = all_tests +1;
		}
		str = "The quick brown fox jumps over the lazy dog";
		vec = bl_str(str);
		sub_str = "The quic";
		vec2 = bl_str(sub_str);
		sub_str2 = "y dog";
		vec3 = bl_str(sub_str2);
		if(declassify(bl_strContains(vec,vec2)) && declassify(bl_strContains(vec,vec3))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! String contains function returned that: ", str, " does not contain: ", sub_str, " or ", sub_str2);
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 14: Index of pattern in string");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p uint result = bl_strIndexOf(vec,vec2);
		}
		string str = "The quick brown fox jumps over the lazy dog";
		pd_a3p xor_uint8[[1]] vec = bl_str(str);
		string sub_str = "ck brown fo";
		pd_a3p xor_uint8[[1]] vec2 = bl_str(sub_str);
		string sub_str2 = "r the laz";
		pd_a3p xor_uint8[[1]] vec3 = bl_str(sub_str2);
		pd_a3p uint index = bl_strIndexOf(vec,vec2);
		pd_a3p uint index2 = bl_strIndexOf(vec,vec3);
		if(declassify(index) == 7 && declassify(index2) == 29){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! String contains function returned that: ", str, " does not contain: ", sub_str, " or ", sub_str2);
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 15: String hamming test");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p uint result = bl_strHamming(vec,vec2);
		}
		string str = "this is a test";
		pd_a3p xor_uint8[[1]] vec = bl_str(str);
		string str2 = "this a test is";
		pd_a3p xor_uint8[[1]] vec2 = bl_str(str2);
		pd_a3p uint hammed = bl_strHamming(vec,vec2);
		if(declassify(hammed) == 8){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Hamming failed. Expected: 8 got: ",declassify(hammed));
	 		all_tests = all_tests +1;
		}
		str = "this is a test";
		vec = bl_str(str);
		str2 = "tset a si siht";
		vec2 = bl_str(str2);
		hammed = bl_strHamming(vec,vec2);
		if(declassify(hammed) == 10){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Hamming failed. Expected: 10 got: ",declassify(hammed));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 16: known string to xor_uint8 vector");
	{
		string test = "The quick brown fox jumps over the lazy dog";
		pd_a3p xor_uint8[[1]] control (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p xor_uint8[[1]] str = kl_str(test);

		if(all(declassify(str == control))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected this: ", arrayToString(declassify(control)) , " got this: ", arrayToString(declassify(str)));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 17: Xor_uint8 vector to known string");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			string result = bl_strDeclassify(vec);
		}
		string test = "The quick brown fox jumps over the lazy dog";
		pd_a3p xor_uint8[[1]] str = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		string test2 = kl_strDeclassify(str);
		if(test == test2){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected this: ", test , " got this: ", test2);
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 18: Known string length function");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			uint length = kl_strLength(str);
		}
		pd_a3p xor_uint8[[1]] str (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		uint length = kl_strLength(str);
		if(length == 43){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected: 43 got: ", length);
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 19: Known strings are equal function");
	{
		{
			pd_a3p xor_uint8[[1]] str (0);
			pd_a3p xor_uint8[[1]] str2 (0);
			pd_a3p bool result = kl_strEquals(str,str2);
		}
		pd_a3p xor_uint8[[1]] str (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p xor_uint8[[1]] str2 (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		if(declassify(kl_strEquals(str,str2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Expected: true got: false");
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 20: Known strings alphabetizing function");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p bool result = kl_strIsLessThan(vec,vec2);
		}
		string str = "This is a test";
		string str2 = "This is also a test";
		pd_a3p xor_uint8[[1]] vec = kl_str(str);
		pd_a3p xor_uint8[[1]] vec2 = kl_str(str2);
		if(declassify(kl_strIsLessThan(vec,vec2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Alphabetizing failed. Expected ", str ," < ", str2);
	 		all_tests = all_tests +1;
		}
		str = "aaaaabbbccdd";
		str2 = "aaaaaaaaaaaa";
		vec = kl_str(str);
		vec2 = kl_str(str2);
		if(declassify(kl_strIsLessThan(vec2,vec))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Alphabetizing failed. Expected ", str2 ," < ", str);
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 21: 2 Known string concatenation");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p xor_uint8[[1]] result = kl_strCat(vec,vec2);
		}
		pd_a3p xor_uint8[[1]] str (21) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106};
		pd_a3p xor_uint8[[1]] str2 (22) = {117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p xor_uint8[[1]] control (43) = {84,104,101,32,113,117,105,99,107,32,98,114,111,119,110,32,102,111,120,32,106,117,109,112,115,32,111,118,101,114,32,116,104,101,32,108,97,122,121,32,100,111,103};
		pd_a3p xor_uint8[[1]] str3 = kl_strCat(str,str2);
		if(all(declassify(str3 == control))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Concatenating strings returned this: ", arrayToString(declassify(str3)), " but expected this: ", arrayToString(declassify(control)));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 22: Known string contains pattern function");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p bool result = kl_strContains(vec,vec2);
		}
		string str = "The quick brown fox jumps over the lazy dog";
		pd_a3p xor_uint8[[1]] vec = kl_str(str);
		string sub_str = "k brown f";
		pd_a3p xor_uint8[[1]] vec2 = kl_str(sub_str);
		string sub_str2 = "mps over th";
		pd_a3p xor_uint8[[1]] vec3 = kl_str(sub_str2);
		if(declassify(kl_strContains(vec,vec2)) && declassify(kl_strContains(vec,vec3))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! String contains function returned that: ", str, " does not contain: ", sub_str, " or ", sub_str2);
	 		all_tests = all_tests +1;
		}
		str = "The quick brown fox jumps over the lazy dog";
		vec = kl_str(str);
		sub_str = "The quic";
		vec2 = kl_str(sub_str);
		sub_str2 = "y dog";
		vec3 = kl_str(sub_str2);
		if(declassify(kl_strContains(vec,vec2)) && declassify(kl_strContains(vec,vec3))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! String contains function returned that: ", str, " does not contain: ", sub_str, " or ", sub_str2);
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 23: Index of pattern in known string");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p uint index = kl_strIndexOf(vec,vec2);
		}
		string str = "The quick brown fox jumps over the lazy dog";
		pd_a3p xor_uint8[[1]] vec = kl_str(str);
		string sub_str = "ck brown fo";
		pd_a3p xor_uint8[[1]] vec2 = kl_str(sub_str);
		string sub_str2 = "r the laz";
		pd_a3p xor_uint8[[1]] vec3 = kl_str(sub_str2);
		pd_a3p uint index = kl_strIndexOf(vec,vec2);
		pd_a3p uint index2 = kl_strIndexOf(vec,vec3);
		if(declassify(index) == 7 && declassify(index2) == 29){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! String contains function returned that: ", str, " does not contain: ", sub_str, " or ", sub_str2);
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 24: String hamming test for known strings");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p uint hammed = kl_strHamming(vec,vec2);
		}
		string str = "this is a test";
		pd_a3p xor_uint8[[1]] vec = kl_str(str);
		string str2 = "this a test is";
		pd_a3p xor_uint8[[1]] vec2 = kl_str(str2);
		pd_a3p uint hammed = kl_strHamming(vec,vec2);
		if(declassify(hammed) == 8){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Hamming failed. Expected: 8 got: ",declassify(hammed));
	 		all_tests = all_tests +1;
		}
		str = "this is a test";
		vec = kl_str(str);
		str2 = "tset a si siht";
		vec2 = kl_str(str2);
		hammed = kl_strHamming(vec,vec2);
		if(declassify(hammed) == 10){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Hamming failed. Expected: 10 got: ",declassify(hammed));
	 		all_tests = all_tests +1;
		}
	}
	print("TEST 25: Levenshtein distance for known string");
	{
		{
			pd_a3p xor_uint8[[1]] vec (0);
			pd_a3p xor_uint8[[1]] vec2 (0);
			pd_a3p uint distance = kl_strLevenshtein(vec,vec2);
		}
		string str = "this is a test";
		string str2 = "these are two tests";
		pd_a3p xor_uint8[[1]] vec = kl_str(str);
		pd_a3p xor_uint8[[1]] vec2 = kl_str(str2);
		pd_a3p uint distance = kl_strLevenshtein(vec,vec2);
		if(declassify(distance) == 9){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! Levenshtein distance failed. Expected 9 but got: " ,declassify(distance));
	 		all_tests = all_tests +1;
		}
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}