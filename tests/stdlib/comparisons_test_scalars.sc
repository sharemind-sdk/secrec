/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module comparisons_test_public_scalars;

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

void main(){
	print("Comparisons test: start");

	print("TEST 1: > operator");
	{
		bool result = true;
		print("bool");
		bool scalar = true;
		if(false > scalar){
			result = false;
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! > operator malfunctioning");
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (4) = true;
		print("uint8");
		uint8 scalar = 90;
		if(60 > scalar || 45 > scalar || 0 > scalar || 89 > scalar || 90 > scalar){
			result[0] = false;
		}
		scalar = 170;
		if(0 > scalar || 1 > scalar || 169 > scalar || 170 > scalar || 55 > scalar || 145 > scalar){
			result[1] = false;
		}
		scalar = 254;
		if(225 > scalar || 0 > scalar || 1 > scalar || 169 > scalar || 253 > scalar || 254 > scalar){
			result[2] = false;
		}
		scalar = 255;
		if(0 > scalar || 255 > scalar || 1 > scalar || 150 > scalar || 254 > scalar){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! > operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}

	}
	{
		bool[[1]] result (4) = true;
		print("uint16");
		uint16 scalar = 2000;
		if(1500 > scalar || 10 > scalar || 0 > scalar || 1999 > scalar || 2000 > scalar){
			result[0] = false;
		}
		scalar = 20786;
		if(0 > scalar || 1 > scalar || 16829 > scalar || 20785 > scalar || 20786 > scalar || 1257 > scalar){
			result[1] = false;
		}
		scalar = 51935;
		if(45862 > scalar || 0 > scalar || 1 > scalar || 51934 > scalar || 51935 > scalar || 12476 > scalar){
			result[2] = false;
		}
		scalar = 65535;
		if(0 > scalar || 65535 > scalar || 1 > scalar || 32658 > scalar || 65534 > scalar){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! > operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = true;
		print("uint32");
		uint32 scalar = 98274;
		if(18755 > scalar || 87632 > scalar || 0 > scalar || 51742 > scalar || 98273 > scalar){
			result[0] = false;
		}
		scalar = 12897533;
		if(0 > scalar || 7283653 > scalar || 11276456 > scalar || 8732658 > scalar || 12897532 > scalar || 5293562 > scalar){
			result[1] = false;
		}
		scalar = 219857621;
		if(219857620 > scalar || 0 > scalar || 63287562 > scalar || 121861285 > scalar || 83275329 > scalar || 1329579 > scalar){
			result[2] = false;
		}
		scalar = 4294967295;
		if(0 > scalar || 4294967295> scalar || 817658234 > scalar || 871265827 > scalar || 4294967294 > scalar){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! > operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = true;
		print("uint64/uint");
		uint scalar = 21576921865219;
		if(21576921865218 > scalar || 78365982165 > scalar || 0 > scalar || 6593826912 > scalar || 26856 > scalar){
			result[0] = false;
		}
		scalar = 72198462159826;
		if(0 > scalar || 72198462159825 > scalar || 87216482 > scalar || 796378983671 > scalar || 2748625876 > scalar || 1353786278 > scalar){
			result[1] = false;
		}
		scalar = 19562189521758219;
		if(1298562187591 > scalar || 0 > scalar || 19562189521758218 > scalar ||  6192856219487289 > scalar || 89274019247891 > scalar || 1 > scalar){
			result[2] = false;
		}
		scalar = 18446744073709551615;
		if(0 > scalar || 18446744073709551614 > scalar || 9812749128742 > scalar || 674789216591269 > scalar || 904732985718629 > scalar){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! > operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int8");
		int8 scalar = 127;
		if(126 > scalar || 50 > scalar || 0 > scalar || -65 > scalar || -23 > scalar){
			result[0] = false;
		}
		scalar = -128;
		if(scalar > 27 || scalar > -25 || scalar > -100 || scalar > 127 || scalar > 96 || scalar > -1){
			result[1] = false;
		}
		scalar = 1;
		if(-1 > scalar || 0 > scalar || -35 > scalar ||  -72 > scalar || -128 > scalar || -121 > scalar){
			result[2] = false;
		}
		scalar = -1;
		if(-45 > scalar || scalar > 0 || scalar > 1 || -62 > scalar || scalar > 99){
			result[3] = false;
		}
		scalar = -50;
		if(scalar > 0 || scalar > 50 || scalar > 127 || -89 > scalar || -128 > scalar){
			result[4] = false;
		}
		scalar = 50;
		if(-25 > scalar || -99 > scalar || 25 > scalar || scalar > 127 || -128 > scalar){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! > operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (6) = true;
		print("int16");
		int16 scalar = 32767;
		if(32766 > scalar || 16732 > scalar || 0 > scalar || -22984 > scalar || -7538 > scalar){
			result[0] = false;
		}
		scalar = -32768;
		if(scalar > -32767 || scalar > -25982 || scalar > -2642 || scalar > 5324|| scalar > 23912 || scalar > 32767){
			result[1] = false;
		}
		scalar = 1;
		if(-1 > scalar || 0 > scalar || -9212 > scalar ||  -32768 > scalar || -31295 > scalar || -3256 > scalar){
			result[2] = false;
		}
		scalar = -1;
		if(-45 > scalar || scalar > 28397 || scalar > 1267 || -8925 > scalar || scalar > 21984){
			result[3] = false;
		}
		scalar = -12856;
		if(scalar > -11782 || scalar > 9812 || scalar > 32767 || -30328 > scalar || -32768 > scalar){
			result[4] = false;
		}
		scalar = 2389;
		if(-25 > scalar || -9217 > scalar || 2189 > scalar || scalar > 32767 || -21568 > scalar){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! > operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int32");
		int32 scalar = 2147483647;
		if(2147483646 > scalar || 187562 > scalar || 0 > scalar || -215869 > scalar || -2147483648 > scalar){
			result[0] = false;
		}
		scalar = -2147483648;
		if(scalar > 27 || scalar > -2498 || scalar > -124698 || scalar > 1289642 || scalar > 2147483647 || scalar > -129875129){
			result[1] = false;
		}
		scalar = 1298749;
		if(-1 > scalar || 0 > scalar || -891269 > scalar ||  -2186 > scalar || -2147483648 > scalar || -218765 > scalar){
			result[2] = false;
		}
		scalar = -129857219;
		if(-218947828 > scalar || scalar > 0 || scalar > 1 || scalar > -28915  || scalar > 2136723765){
			result[3] = false;
		}
		scalar = -1643872691;
		if(scalar > 0 || scalar > 27592367 || scalar > 1876492086 || -1789267491 > scalar || -2147483646 > scalar){
			result[4] = false;
		}
		scalar = 1298573027;
		if(-25 > scalar || -12895 > scalar || 127953 > scalar || scalar > 2147483647 || -2147483648 > scalar){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! > operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int64/int");
		int scalar = 9223372036854775807;
		if(2198742159862154 > scalar || 2965325 > scalar || 0 > scalar || -2157612985 > scalar || -4129805298 > scalar){
			result[0] = false;
		}
		scalar = -9223372036854775808;
		if(scalar > -2598362598 || scalar > -22910752 || scalar > -192865219876 || scalar > 32985623598 || scalar > 2187652198678 || scalar > 988765219875628){
			result[1] = false;
		}
		scalar = 1;
		if(-1 > scalar || 0 > scalar || -92903276592 > scalar ||  -765328568 > scalar || -9223372036854775808 > scalar || -39857329865 > scalar){
			result[2] = false;
		}
		scalar = -1;
		if(-9223372036854775808 > scalar || scalar > 2198561298462 || scalar > 9223372036854775807 || -129832598 > scalar || scalar > 2398652396149){
			result[3] = false;
		}
		scalar = -52398573219712;
		if(scalar > -5528917289724 || scalar > 21895621985 || scalar > 321752767 || -9223372036854775808 > scalar || -532219852036854773 > scalar){
			result[4] = false;
		}
		scalar = 193286529385326552;
		if(-25 > scalar || -923876510987 > scalar || 218923896723 > scalar || scalar > 9223372036854775807 || -9223372036854775808 > scalar){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! > operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}	
	}	
	print("TEST 2: < operator");
	{
		bool result = true;
		print("bool");
		bool scalar = false;
		if(true < scalar){
			result = false;
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! < operator malfunctioning");
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (4) = true;
		print("uint8");
		uint8 scalar = 90;
		if(scalar < 60 ||  scalar < 45 ||  scalar < 0  ||  scalar < 89 ||  scalar < 90 ){
			result[0] = false;
		}
		scalar = 170;
		if( scalar < 0  ||  scalar < 1   ||  scalar <  169 || scalar < 170 ||  scalar < 55 || scalar < 145 ){
			result[1] = false;
		}
		scalar = 254;
		if( scalar < 225  ||  scalar < 0 ||  scalar < 1  ||  scalar < 169  ||  scalar < 253  ||  scalar < 254 ){
			result[2] = false;
		}
		scalar = 255;
		if( scalar < 0 ||  scalar < 255 ||  scalar < 1 ||  scalar < 150 ||  scalar < 254){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! < operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}

	}
	{
		bool[[1]] result (4) = true;
		print("uint16");
		uint16 scalar = 2000;
		if( scalar < 1500 ||  scalar < 10  ||  scalar < 0 ||  scalar < 1999  ||  scalar < 2000 ){
			result[0] = false;
		}
		scalar = 20786;
		if(scalar < 0  ||  scalar < 1  ||  scalar < 16829 ||  scalar < 20785  || scalar < 20786  ||  scalar < 1257 ){
			result[1] = false;
		}
		scalar = 51935;
		if(scalar < 45862  || scalar < 0  ||  scalar < 1  ||  scalar < 51934  ||  scalar < 51935 ||  scalar < 12476){
			result[2] = false;
		}
		scalar = 65535;
		if( scalar < 0 ||  scalar < 65535 ||  scalar < 1 ||  scalar < 32658 ||  scalar < 65534){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! < operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = true;
		print("uint32");
		uint32 scalar = 98274;
		if( scalar < 18755 ||  scalar < 87632  ||  scalar < 0 ||  scalar < 51742 ||  scalar < 98273){
			result[0] = false;
		}
		scalar = 12897533;
		if( scalar < 0  ||  scalar < 7283653  ||  scalar < 11276456 ||  scalar < 8732658  ||  scalar < 12897532  ||  scalar < 5293562 ){
			result[1] = false;
		}
		scalar = 219857621;
		if( scalar < 219857620 ||  scalar < 0 || scalar < 63287562 || scalar < 121861285 || scalar < 83275329 || scalar < 1329579){
			result[2] = false;
		}
		scalar = 4294967295;
		if(scalar < 0 ||  scalar < 4294967295 ||  scalar < 817658234  ||  scalar < 871265827 ||  scalar < 4294967294){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! < operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = true;
		print("uint64/uint");
		uint scalar = 21576921865219;
		if( scalar < 21576921865218 ||  scalar < 78365982165 ||  scalar < 0 ||  scalar < 6593826912 || scalar < 26856){
			result[0] = false;
		}
		scalar = 72198462159826;
		if( scalar < 0  ||  scalar < 72198462159825 || scalar < 87216482 || scalar < 796378983671 || scalar < 2748625876  ||  scalar < 1353786278){
			result[1] = false;
		}
		scalar = 19562189521758219;
		if(scalar < 1298562187591  ||  scalar < 0 ||  scalar < 19562189521758218 ||  scalar <  6192856219487289 || scalar < 89274019247891 || scalar < 1){
			result[2] = false;
		}
		scalar = 18446744073709551615;
		if(scalar < 0 || scalar < 18446744073709551614 || scalar < 9812749128742 || scalar < 674789216591269 || scalar < 904732985718629){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! < operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int8");
		int8 scalar = 127;
		if( scalar < 126 || scalar < 50 ||  scalar < 0 ||  scalar < -65 || scalar < -23){
			result[0] = false;
		}
		scalar = -128;
		if( 27 < scalar || -25 < scalar ||  -100 < scalar || 127 < scalar|| 96 < scalar ||-1 <  scalar){
			result[1] = false;
		}
		scalar = 1;
		if( scalar < -1 || scalar < 0 || scalar < -35 || scalar <  -72  || scalar < -128 || scalar < -121){
			result[2] = false;
		}
		scalar = -1;
		if(scalar < -45 || 0 < scalar|| 1 < scalar || scalar < -62 || 99 < scalar){
			result[3] = false;
		}
		scalar = -50;
		if( 0 < scalar || 50 < scalar || 127 < scalar || scalar <  -89 || scalar < -128){
			result[4] = false;
		}
		scalar = 50;
		if( scalar < -25 || scalar < -99 || scalar < 25|| 127 < scalar ||scalar <  -128){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! < operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (6) = true;
		print("int16");
		int16 scalar = 32767;
		if( scalar < 32766 ||  scalar < 16732 ||  scalar < 0 ||  scalar < -22984 || scalar < -7538){
			result[0] = false;
		}
		scalar = -32768;
		if(-32767 < scalar ||-25982 < scalar || -2642 < scalar || 5324 < scalar|| 23912 < scalar  || 32767 < scalar){
			result[1] = false;
		}
		scalar = 1;
		if( scalar < -1 || scalar < 0 || scalar < -9212 || scalar <  -32768 || scalar < -31295 || scalar < -3256){
			result[2] = false;
		}
		scalar = -1;
		if( scalar < -45 ||28397 <  scalar || 1267 < scalar || scalar <  -8925 || 21984 <  scalar){
			result[3] = false;
		}
		scalar = -12856;
		if( -11782 < scalar ||  9812 < scalar || 32767 < scalar || scalar <  -30328 || scalar < -32768){
			result[4] = false;
		}
		scalar = 2389;
		if( scalar < -25 || scalar < -9217 || scalar < 2189 || 32767 < scalar || scalar < -21568){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! < operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int32");
		int32 scalar = 2147483647;
		if( scalar < 2147483646 || scalar <  187562 || scalar <  0 || scalar <  -215869 || scalar <  -2147483648){
			result[0] = false;
		}
		scalar = -2147483648;
		if( 27 < scalar || -2498 < scalar  || -124698 < scalar ||  1289642 < scalar || 2147483647 < scalar || -129875129 < scalar){
			result[1] = false;
		}
		scalar = 1298749;
		if(scalar < -1 || scalar < 0 || scalar < -891269 || scalar < -2186 || scalar < -2147483648 || scalar <  -218765){
			result[2] = false;
		}
		scalar = -129857219;
		if( scalar < -218947828 || 0 <  scalar || 1 < scalar ||-28915 < scalar || 2136723765 < scalar){
			result[3] = false;
		}
		scalar = -1643872691;
		if(0 < scalar || 27592367 < scalar || 1876492086 < scalar || scalar <  -1789267491 || scalar < -2147483646){
			result[4] = false;
		}
		scalar = 1298573027;
		if(scalar < -25 || scalar < -12895 || scalar < 127953 || 2147483647 < scalar || scalar < -2147483648){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! < operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int64/int");
		int scalar = 9223372036854775807;
		if( scalar < 2198742159862154 || scalar < 2965325 || scalar < 0 || scalar < -2157612985 || scalar <  -4129805298){
			result[0] = false;
		}
		scalar = -9223372036854775808;
		if(-2598362598 < scalar || -22910752 < scalar || -192865219876 < scalar || 32985623598 < scalar || 2187652198678 < scalar || 988765219875628 < scalar){
			result[1] = false;
		}
		scalar = 1;
		if( scalar < -1 || scalar < 0 || scalar <  -92903276592 || scalar < -765328568 || scalar < -9223372036854775808 ||  scalar < -39857329865){
			result[2] = false;
		}
		scalar = -1;
		if( scalar < -9223372036854775808 || 2198561298462 < scalar || 9223372036854775807 < scalar || scalar < -129832598 ||  2398652396149 < scalar){
			result[3] = false;
		}
		scalar = -52398573219712;
		if(-5528917289724 < scalar || 21895621985 < scalar|| 321752767 < scalar || scalar < -9223372036854775808 ||  scalar < -532219852036854773){
			result[4] = false;
		}
		scalar = 193286529385326552;
		if(scalar < -25 || scalar < -923876510987 || scalar < 218923896723 || 9223372036854775807 < scalar || scalar < -9223372036854775808){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! < operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}

	print("TEST 3: >= operator");
	{
		bool result = true;
		print("bool");
		bool scalar = true;
		if(false >= scalar){
			result = false;
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! >= operator malfunctioning");
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (4) = true;
		print("uint8");
		uint8 scalar = 90;
		if(60 >= scalar || 45 >= scalar || 0 >= scalar || 89 >= scalar || 27 >= scalar){
			result[0] = false;
		}
		scalar = 170;
		if(0 >= scalar || 1 >= scalar || 169 >= scalar || 65 >= scalar || 55 >= scalar || 145 >= scalar){
			result[1] = false;
		}
		scalar = 254;
		if(225 >= scalar || 0 >= scalar || 1 >= scalar || 169 >= scalar || 253 >= scalar || 56 >= scalar){
			result[2] = false;
		}
		scalar = 255;
		if(0 >= scalar || 254 >= scalar || 1 >= scalar || 150 >= scalar || 201 >= scalar){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! >= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}

	}
	{
		bool[[1]] result (4) = true;
		print("uint16");
		uint16 scalar = 2000;
		if(1500 >= scalar || 10 >= scalar || 0 >= scalar || 1999 >= scalar || 325 >= scalar){
			result[0] = false;
		}
		scalar = 20786;
		if(0 >= scalar || 1 >= scalar || 16829 >= scalar || 20785 >= scalar || 9879 >= scalar || 1257 >= scalar){
			result[1] = false;
		}
		scalar = 51935;
		if(45862 >= scalar || 0 >= scalar || 1 >= scalar || 51934 >= scalar || 22598 >= scalar || 12476 >= scalar){
			result[2] = false;
		}
		scalar = 65535;
		if(0 >= scalar || 42987 >= scalar || 1 >= scalar || 32658 >= scalar || 65534 >= scalar){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! >= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = true;
		print("uint32");
		uint32 scalar = 98274;
		if(18755 >= scalar || 87632 >= scalar || 0 >= scalar || 51742 >= scalar || 98273 >= scalar){
			result[0] = false;
		}
		scalar = 12897533;
		if(0 >= scalar || 7283653 >= scalar || 11276456 >= scalar || 8732658 >= scalar || 12897532 >= scalar || 5293562 >= scalar){
			result[1] = false;
		}
		scalar = 219857621;
		if(219857620 >= scalar || 0 >= scalar || 63287562 >= scalar || 121861285 >= scalar || 83275329 >= scalar || 1329579 >= scalar){
			result[2] = false;
		}
		scalar = 4294967295;
		if(0 >= scalar || 429453 >= scalar || 817658234 >= scalar || 871265827 >= scalar || 4294967294 >= scalar){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! >= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = true;
		print("uint64/uint");
		uint scalar = 21576921865219;
		if(21576921865218 >= scalar || 78365982165 >= scalar || 0 >= scalar || 6593826912 >= scalar || 26856 >= scalar){
			result[0] = false;
		}
		scalar = 72198462159826;
		if(0 >= scalar || 72198462159825 >= scalar || 87216482 >= scalar || 796378983671 >= scalar || 2748625876 >= scalar || 1353786278 >= scalar){
			result[1] = false;
		}
		scalar = 19562189521758219;
		if(1298562187591 >= scalar || 0 >= scalar || 19562189521758218 >= scalar ||  6192856219487289 >= scalar || 89274019247891 >= scalar || 1 >= scalar){
			result[2] = false;
		}
		scalar = 18446744073709551615;
		if(0 >= scalar || 18446744073709551614 >= scalar || 9812749128742 >= scalar || 674789216591269 >= scalar || 904732985718629 >= scalar){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! >= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int8");
		int8 scalar = 127;
		if(126 >= scalar || 50 >= scalar || 0 >= scalar || -65 >= scalar || -23 >= scalar){
			result[0] = false;
		}
		scalar = -128;
		if(scalar >= 27 || scalar >= -25 || scalar >= -100 || scalar >= 127 || scalar >= 96 || scalar >= -1){
			result[1] = false;
		}
		scalar = 1;
		if(-1 >= scalar || 0 >= scalar || -35 >= scalar ||  -72 >= scalar || -128 >= scalar || -121 >= scalar){
			result[2] = false;
		}
		scalar = -1;
		if(-45 >= scalar || scalar >= 0 || scalar >= 1 || -62 >= scalar || scalar >= 99){
			result[3] = false;
		}
		scalar = -50;
		if(scalar >= 0 || scalar >= 50 || scalar >= 127 || -89 >= scalar || -128 >= scalar){
			result[4] = false;
		}
		scalar = 50;
		if(-25 >= scalar || -99 >= scalar || 25 >= scalar || scalar >= 127 || -128 >= scalar){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! >= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (6) = true;
		print("int16");
		int16 scalar = 32767;
		if(32766 >= scalar || 16732 >= scalar || 0 >= scalar || -22984 >= scalar || -7538 >= scalar){
			result[0] = false;
		}
		scalar = -32768;
		if(scalar >= -32767 || scalar >= -25982 || scalar >= -2642 || scalar >= 5324|| scalar >= 23912 || scalar >= 32767){
			result[1] = false;
		}
		scalar = 1;
		if(-1 >= scalar || 0 >= scalar || -9212 >= scalar ||  -32768 >= scalar || -31295 >= scalar || -3256 >= scalar){
			result[2] = false;
		}
		scalar = -1;
		if(-45 >= scalar || scalar >= 28397 || scalar >= 1267 || -8925 >= scalar || scalar >= 21984){
			result[3] = false;
		}
		scalar = -12856;
		if(scalar >= -11782 || scalar >= 9812 || scalar >= 32767 || -30328 >= scalar || -32768 >= scalar){
			result[4] = false;
		}
		scalar = 2389;
		if(-25 >= scalar || -9217 >= scalar || 2189 >= scalar || scalar >= 32767 || -21568 >= scalar){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! >= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int32");
		int32 scalar = 2147483647;
		if(2147483646 >= scalar || 187562 >= scalar || 0 >= scalar || -215869 >= scalar || -2147483648 >= scalar){
			result[0] = false;
		}
		scalar = -2147483648;
		if(scalar >= 27 || scalar >= -2498 || scalar >= -124698 || scalar >= 1289642 || scalar >= 2147483647 || scalar >= -129875129){
			result[1] = false;
		}
		scalar = 1298749;
		if(-1 >= scalar || 0 >= scalar || -891269 >= scalar ||  -2186 >= scalar || -2147483648 >= scalar || -218765 >= scalar){
			result[2] = false;
		}
		scalar = -129857219;
		if(-218947828 >= scalar || scalar >= 0 || scalar >= 1 || scalar >= -28915  || scalar >= 2136723765){
			result[3] = false;
		}
		scalar = -1643872691;
		if(scalar >= 0 || scalar >= 27592367 || scalar >= 1876492086 || -1789267491 >= scalar || -2147483646 >= scalar){
			result[4] = false;
		}
		scalar = 1298573027;
		if(-25 >= scalar || -12895 >= scalar || 127953 >= scalar || scalar >= 2147483647 || -2147483648 >= scalar){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! >= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int64/int");
		int scalar = 9223372036854775807;
		if(2198742159862154 >= scalar || 2965325 >= scalar || 0 >= scalar || -2157612985 >= scalar || -4129805298 >= scalar){
			result[0] = false;
		}
		scalar = -9223372036854775808;
		if(scalar >= -2598362598 || scalar >= -22910752 || scalar >= -192865219876 || scalar >= 32985623598 || scalar >= 2187652198678 || scalar >= 988765219875628){
			result[1] = false;
		}
		scalar = 1;
		if(-1 >= scalar || 0 >= scalar || -92903276592 >= scalar ||  -765328568 >= scalar || -9223372036854775808 >= scalar || -39857329865 >= scalar){
			result[2] = false;
		}
		scalar = -1;
		if(-9223372036854775808 >= scalar || scalar >= 2198561298462 || scalar >= 9223372036854775807 || -129832598 >= scalar || scalar >= 2398652396149){
			result[3] = false;
		}
		scalar = -52398573219712;
		if(scalar >= -5528917289724 || scalar >= 21895621985 || scalar >= 321752767 || -9223372036854775808 >= scalar || -532219852036854773 >= scalar){
			result[4] = false;
		}
		scalar = 193286529385326552;
		if(-25 >= scalar || -923876510987 >= scalar || 218923896723 >= scalar || scalar >= 9223372036854775807 || -9223372036854775808 >= scalar){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! >= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}	
	}
	print("TEST 4: <= operator");
	{
		bool result = true;
		print("bool");
		bool scalar = false;
		if(true <= scalar){
			result = false;
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! <= operator malfunctioning");
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (4) = true;
		print("uint8");
		uint8 scalar = 90;
		if(scalar <= 60 ||  scalar <= 45 ||  scalar <= 0  ||  scalar <= 89 ||  scalar <= 67 ){
			result[0] = false;
		}
		scalar = 170;
		if( scalar <= 0  ||  scalar <= 1   ||  scalar <=  169 || scalar <= 15 ||  scalar <= 55 || scalar <= 145 ){
			result[1] = false;
		}
		scalar = 254;
		if( scalar <= 225  ||  scalar <= 0 ||  scalar <= 1  ||  scalar <= 169  ||  scalar <= 253  ||  scalar <= 45 ){
			result[2] = false;
		}
		scalar = 255;
		if( scalar <= 0 ||  scalar <= 134 ||  scalar <= 1 ||  scalar <= 150 ||  scalar <= 254){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! <= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}

	}
	{
		bool[[1]] result (4) = true;
		print("uint16");
		uint16 scalar = 2000;
		if( scalar <= 1500 ||  scalar <= 10  ||  scalar <= 0 ||  scalar <= 1999  ||  scalar <= 256 ){
			result[0] = false;
		}
		scalar = 20786;
		if(scalar <= 0  ||  scalar <= 1  ||  scalar <= 16829 ||  scalar <= 20785  || scalar <= 14657  ||  scalar <= 1257 ){
			result[1] = false;
		}
		scalar = 51935;
		if(scalar <= 45862  || scalar <= 0  ||  scalar <= 1  ||  scalar <= 51934  ||  scalar <= 43618 ||  scalar <= 12476){
			result[2] = false;
		}
		scalar = 65535;
		if( scalar <= 0 ||  scalar <= 51908 ||  scalar <= 1 ||  scalar <= 32658 ||  scalar <= 65534){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! <= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = true;
		print("uint32");
		uint32 scalar = 98274;
		if( scalar <= 18755 ||  scalar <= 87632  ||  scalar <= 0 ||  scalar <= 51742 ||  scalar <= 98273){
			result[0] = false;
		}
		scalar = 12897533;
		if( scalar <= 0  ||  scalar <= 7283653  ||  scalar <= 11276456 ||  scalar <= 8732658  ||  scalar <= 12897532  ||  scalar <= 5293562 ){
			result[1] = false;
		}
		scalar = 219857621;
		if( scalar <= 219857620 ||  scalar <= 0 || scalar <= 63287562 || scalar <= 121861285 || scalar <= 83275329 || scalar <= 1329579){
			result[2] = false;
		}
		scalar = 4294967295;
		if(scalar <= 0 ||  scalar <= 42943253 ||  scalar <= 817658234  ||  scalar <= 871265827 ||  scalar <= 4294967294){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! <= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = true;
		print("uint64/uint");
		uint scalar = 21576921865219;
		if( scalar <= 21576921865218 ||  scalar <= 78365982165 ||  scalar <= 0 ||  scalar <= 6593826912 || scalar <= 26856){
			result[0] = false;
		}
		scalar = 72198462159826;
		if( scalar <= 0  ||  scalar <= 72198462159825 || scalar <= 87216482 || scalar <= 796378983671 || scalar <= 2748625876  ||  scalar <= 1353786278){
			result[1] = false;
		}
		scalar = 19562189521758219;
		if(scalar <= 1298562187591  ||  scalar <= 0 ||  scalar <= 19562189521758218 ||  scalar <=  6192856219487289 || scalar <= 89274019247891 || scalar <= 1){
			result[2] = false;
		}
		scalar = 18446744073709551615;
		if(scalar <= 0 || scalar <= 18446744073709551614 || scalar <= 9812749128742 || scalar <= 674789216591269 || scalar <= 904732985718629){
			result[3] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! <= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int8");
		int8 scalar = 127;
		if( scalar <= 126 || scalar <= 50 ||  scalar <= 0 ||  scalar <= -65 || scalar <= -23){
			result[0] = false;
		}
		scalar = -128;
		if( 27 <= scalar || -25 <= scalar ||  -100 <= scalar || 127 <= scalar|| 96 <= scalar ||-1 <=  scalar){
			result[1] = false;
		}
		scalar = 1;
		if( scalar <= -1 || scalar <= 0 || scalar <= -35 || scalar <=  -72  || scalar <= -128 || scalar <= -121){
			result[2] = false;
		}
		scalar = -1;
		if(scalar <= -45 || 0 <= scalar|| 1 <= scalar || scalar <= -62 || 99 <= scalar){
			result[3] = false;
		}
		scalar = -50;
		if( 0 <= scalar || 50 <= scalar || 127 <= scalar || scalar <=  -89 || scalar <= -128){
			result[4] = false;
		}
		scalar = 50;
		if( scalar <= -25 || scalar <= -99 || scalar <= 25|| 127 <= scalar ||scalar <=  -128){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! <= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (6) = true;
		print("int16");
		int16 scalar = 32767;
		if( scalar <= 32766 ||  scalar <= 16732 ||  scalar <= 0 ||  scalar <= -22984 || scalar <= -7538){
			result[0] = false;
		}
		scalar = -32768;
		if(-32767 <= scalar ||-25982 <= scalar || -2642 <= scalar || 5324 <= scalar|| 23912 <= scalar  || 32767 <= scalar){
			result[1] = false;
		}
		scalar = 1;
		if( scalar <= -1 || scalar <= 0 || scalar <= -9212 || scalar <=  -32768 || scalar <= -31295 || scalar <= -3256){
			result[2] = false;
		}
		scalar = -1;
		if( scalar <= -45 ||28397 <=  scalar || 1267 <= scalar || scalar <=  -8925 || 21984 <=  scalar){
			result[3] = false;
		}
		scalar = -12856;
		if( -11782 <= scalar ||  9812 <= scalar || 32767 <= scalar || scalar <=  -30328 || scalar <= -32768){
			result[4] = false;
		}
		scalar = 2389;
		if( scalar <= -25 || scalar <= -9217 || scalar <= 2189 || 32767 <= scalar || scalar <= -21568){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! <= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int32");
		int32 scalar = 2147483647;
		if( scalar <= 2147483646 || scalar <=  187562 || scalar <=  0 || scalar <=  -215869 || scalar <=  -2147483648){
			result[0] = false;
		}
		scalar = -2147483648;
		if( 27 <= scalar || -2498 <= scalar  || -124698 <= scalar ||  1289642 <= scalar || 2147483647 <= scalar || -129875129 <= scalar){
			result[1] = false;
		}
		scalar = 1298749;
		if(scalar <= -1 || scalar <= 0 || scalar <= -891269 || scalar <= -2186 || scalar <= -2147483648 || scalar <=  -218765){
			result[2] = false;
		}
		scalar = -129857219;
		if( scalar <= -218947828 || 0 <=  scalar || 1 <= scalar ||-28915 <= scalar || 2136723765 <= scalar){
			result[3] = false;
		}
		scalar = -1643872691;
		if(0 <= scalar || 27592367 <= scalar || 1876492086 <= scalar || scalar <=  -1789267491 || scalar <= -2147483646){
			result[4] = false;
		}
		scalar = 1298573027;
		if(scalar <= -25 || scalar <= -12895 || scalar <= 127953 || 2147483647 <= scalar || scalar <= -2147483648){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! <= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = true;
		print("int64/int");
		int scalar = 9223372036854775807;
		if( scalar <= 2198742159862154 || scalar <= 2965325 || scalar <= 0 || scalar <= -2157612985 || scalar <=  -4129805298){
			result[0] = false;
		}
		scalar = -9223372036854775808;
		if(-2598362598 <= scalar || -22910752 <= scalar || -192865219876 <= scalar || 32985623598 <= scalar || 2187652198678 <= scalar || 988765219875628 <= scalar){
			result[1] = false;
		}
		scalar = 1;
		if( scalar <= -1 || scalar <= 0 || scalar <=  -92903276592 || scalar <= -765328568 || scalar <= -9223372036854775808 ||  scalar <= -39857329865){
			result[2] = false;
		}
		scalar = -1;
		if( scalar <= -9223372036854775808 || 2198561298462 <= scalar || 9223372036854775807 <= scalar || scalar <= -129832598 ||  2398652396149 <= scalar){
			result[3] = false;
		}
		scalar = -52398573219712;
		if(-5528917289724 <= scalar || 21895621985 <= scalar|| 321752767 <= scalar || scalar <= -9223372036854775808 ||  scalar <= -532219852036854773){
			result[4] = false;
		}
		scalar = 193286529385326552;
		if(scalar <= -25 || scalar <= -923876510987 || scalar <= 218923896723 || 9223372036854775807 <= scalar || scalar <= -9223372036854775808){
			result[5] = false;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! <= operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}	
	}
	print("TEST 5: == operator");
	{
		bool result = false;
		print("bool");
		bool scalar = true;
		if(true == scalar){
			result = true;
		}
		if(result){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! == operator malfunctioning");
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (4) = false;
		print("uint8");
	
		if(60 == 60 && 45 == 45 && 0 == 0 && 89 == 89){
			result[0] = true;
		}
		if(0 == 0 && 1 == 1 && 169 == 169 && 55 == 55 && 145 == 145){
			result[1] = true;
		}	
		if(225 == 225 && 0 == 0 && 1 == 1 && 169 == 169 && 253 == 253){
			result[2] = true;
		}
		if(0 == 0 && 1 == 1 && 150 == 150 && 254 == 254){
			result[3] = true;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! == operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}

	}
	{
		bool[[1]] result (4) = false;
		print("uint16");
		if(1500 == 1500 && 10 == 10 && 0 == 0 && 1999 == 1999){
			result[0] = true;
		}
		if(0 == 0 && 1 == 1 && 16829 == 16829 && 20785 == 20785 && 1257 == 1257){
			result[1] = true;
		}
		if(45862 == 45862 && 0 == 0 && 1 == 1 && 51934 == 51934 && 12476 == 12476){
			result[2] = true;
		}
		if(0 == 0 && 1 == 1 && 32658 == 32658 && 65534 == 65534){
			result[3] = true;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! == operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = false;
		print("uint32");
		if(18755 == 18755 && 87632 == 87632 && 0 == 0 && 51742 == 51742 && 98273 == 98273){
			result[0] = true;
		}
		if(0 == 0 && 7283653 == 7283653 && 11276456 == 11276456 && 8732658 == 8732658 && 12897532 == 12897532 && 5293562 == 5293562){
			result[1] = true;
		}
		if(219857620 == 219857620 && 0 == 0 && 63287562 == 63287562 && 121861285 == 121861285 && 83275329 == 83275329 && 1329579 == 1329579){
			result[2] = true;
		}
		if(0 == 0 && 817658234 == 817658234 && 871265827 == 871265827 && 4294967294 == 4294967294){
			result[3] = true;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! == operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (4) = false;
		print("uint64/uint");
		if(21576921865218 == 21576921865218 && 78365982165 == 78365982165 && 0 == 0 && 6593826912 == 6593826912 && 26856 == 26856){
			result[0] = true;
		}
		if(0 == 0 && 72198462159825 == 72198462159825 && 87216482 == 87216482 && 796378983671 == 796378983671 && 2748625876 == 2748625876 && 1353786278 == 1353786278){
			result[1] = true;
		}	
		if(1298562187591 == 1298562187591 && 0 == 0 && 19562189521758218 == 19562189521758218 &&  6192856219487289 == 6192856219487289 && 89274019247891 == 89274019247891 && 1 == 1){
			result[2] = true;
		}
		if(0 == 0 && 18446744073709551614 == 18446744073709551614 && 9812749128742 == 9812749128742 && 674789216591269 == 674789216591269 && 904732985718629 == 904732985718629){
			result[3] = true;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! == operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = false;
		print("int8");
		if(126 == 126 && 50 == 50 && 0 == 0 && -65 == -65 && -23 == -23){
			result[0] = true;
		}
		if(27 == 27 && -25 == -25 && -100 == -100 && 127 == 127 && 96 == 96 && -1 == -1){
			result[1] = true;
		}
		if(-1 == -1 && 0 == 0 && -35 == -35 &&  -72 == -72 && -128 == -128  && -121 == -121){
			result[2] = true;
		}
		if(-45 == -45 && 0 == 0 && 1 == 1 && -62 == -62 && 99 == 99){
			result[3] = true;
		}
		if(0 == 0 && 50 == 50 && 127 == 127 && -89 == -89 && -128 == -128){
			result[4] = true;
		}
		if(-25 == -25 && -99 == -99 && 25 == 25 && 127 == 127 && -128 == -128){
			result[5] = true;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! == operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}
	{
		bool[[1]] result (6) = false;
		print("int16");
		
		if(32766 == 32766 && 16732 == 16732 && 0 == 0 && -22984 == -22984 && -7538 == -7538){
			result[0] = true;
		}
		if(-32767 == -32767 && -25982 == -25982 && -2642 == -2642 && 5324 == 5324&& 23912 == 23912 && 32767 == 32767){
			result[1] = true;
		}
		if(-1 == -1 && 0 == 0 && -9212 == -9212 &&  -32768 == -32768 && -31295 == -31295 && -3256 == -3256){
			result[2] = true;
		}
		if(-45 == -45 && 28397 == 28397 && 1267 == 1267 && -8925 == -8925 && 21984 == 21984){
			result[3] = true;
		}	
		if(-11782  == -11782 && 9812 == 9812 && 32767 == 32767 && -30328 == -30328 && -32768 == -32768){
			result[4] = true;
		}
		if(-25 == -25 && -9217 == -9217 && 2189 == 2189 && 32767 == 32767 && -21568 == -21568){
			result[5] = true;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! == operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = false;
		print("int32");
		if(2147483646 == 2147483646 && 187562 == 187562 && 0 == 0 && -215869 == -215869 && -2147483648 == -2147483648){
			result[0] = true;
		}
		if(27 == 27 && -2498 == -2498 && -124698 == -124698 && 1289642 == 1289642 && 2147483647 == 2147483647 && -129875129 == -129875129){
			result[1] = true;
		}		
		if(-1 == -1 && 0 == 0 && -891269 == -891269 &&  -2186 == -2186 && -2147483648 == -2147483648 && -218765 == -218765){
			result[2] = true;
		}		
		if(-218947828 == -218947828 && 0 == 0 && 1 == 1 && -28915 == -28915  && 2136723765 == 2136723765){
			result[3] = true;
		}	
		if(0 == 0 && 27592367 == 27592367 && 1876492086 == 1876492086 && -1789267491 == -1789267491 && -2147483646 == -2147483646){
			result[4] = true;
		}
		if(-25 == -25 && -12895 == -12895 && 127953 == 127953 && 2147483647 == 2147483647 && -2147483648 == -2147483648){
			result[5] = true;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! == operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
	}
	{
		bool[[1]] result (6) = false;
		print("int64/int");
		if(2198742159862154 == 2198742159862154 && 2965325 == 2965325 && 0 == 0 && -2157612985 == -2157612985 && -4129805298 == -4129805298){
			result[0] = true;
		}
		if(-2598362598 == -2598362598 && -22910752 == -22910752 && -192865219876 == -192865219876 && 32985623598 == 32985623598 && 2187652198678 == 2187652198678 && 988765219875628 == 988765219875628){
			result[1] = true;
		}
		if(-1 == -1 && 0 == 0 && -92903276592 == -92903276592 &&  -765328568 == -765328568 && -9223372036854775808 == -9223372036854775808 && -39857329865 == -39857329865){
			result[2] = true;
		}
		if(-9223372036854775808 == -9223372036854775808 && 2198561298462 == 2198561298462 && 9223372036854775807 == 9223372036854775807 && -129832598 == -129832598 && 2398652396149 == 2398652396149){
			result[3] = true;
		}
		if(-5528917289724 == -5528917289724 && 21895621985 == 21895621985 && 321752767 == 321752767 && -9223372036854775808 == -9223372036854775808 && -532219852036854773 == -532219852036854773){
			result[4] = true;
		}
		if(-25 == -25 && -923876510987 == -923876510987  && 218923896723 == 218923896723 && 9223372036854775807 == 9223372036854775807 && -9223372036854775808 == -9223372036854775808){
			result[5] = true;
		}
		if(all(result)){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			print("FAILURE! == operator malfunctioning", arrayToString(result));
	 		all_tests = all_tests +1;
 		}
		
	}	

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}