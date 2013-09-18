/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module miscellaneous_test;

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

template<type T>
T random_float(T data){
    T rand = 1;
    for(uint i = 0; i < 2; ++i){
        pd_a3p uint32 temp;
        pd_a3p int8 temp2;
        while(declassify(temp) == 0 || declassify(temp2) == 0){
            temp = randomize(temp);
            temp2 = randomize(temp2);
        }
        T scalar = (T) declassify(temp);
        T scalar2 = (T) declassify(temp2);
        if((i % 2) == 0){
            rand *= scalar;
            rand *= scalar2;
        }
        else{
            rand /= scalar;
            rand /= scalar2;
        }
    }
    return rand;
}

template<domain D: additive3pp,type T>
D T[[1]] random(D T[[1]] data){
    uint x_shape = shape(data)[0];
    T[[1]] temp (x_shape);
    for(uint i = 0; i < x_shape;++i){
        temp[i] = random_float(0::T);
    }
    D T[[1]] result = temp;
    return result;
}

template<domain D:additive3pp,type T>
void cast_bool_xor(D T data){
	//private
	bool result = true;
	pd_a3p bool[[1]] temp (5);
	temp = randomize(temp);
	pd_a3p T[[1]] c (5) = (T)temp;
	for(uint i = 0; i < 5; ++i){
		if(declassify(temp[i]) == true && declassify(c[i]) == 0){
			result = false;
		}
		if(declassify(temp[i]) == false && declassify(c[i]) == 1){
			result = false;
		}
	}
	if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
	}
	else{
		print("SUCCESS!");
		succeeded_tests +=1;
		all_tests += 1;
	}
}

void main(){

	print("Miscellaneous test: start");

	print("TEST 2: casting values");
	{
		print("bool -> xor_uint8");
		pd_a3p xor_uint8 data = 0;
		cast_bool_xor(data);
	}
	{
		print("bool -> xor_uint16");
		pd_a3p xor_uint16 data = 0;
		cast_bool_xor(data);
	}
	{
		print("bool -> xor_uint32");
		pd_a3p xor_uint32 data = 0;
		cast_bool_xor(data);
	}
	{
		print("bool -> xor_uint");
		pd_a3p xor_uint64 data = 0;
		cast_bool_xor(data);
	}
	print("TEST 4: casting values(3)");
	{
		print("uint8 -> int");
		bool result = true;
		pd_a3p uint8[[1]] a = {0,100,200,255};
		{
			//int8
			pd_a3p int8[[1]] b = (int8)(a::uint8);
			int8[[1]] control = {0,100,200,255};
			if(any(declassify(b) != control)){result = false;}
			pd_a3p uint8[[1]] temp = a;
			pd_a3p int8[[1]] c = (int8)(temp::uint8);
			if(any(declassify(c) != control)){result = false;}
		}
		{
			//int16
			pd_a3p int16[[1]] b = (int16)(a::uint8);
			int16[[1]] control = {0,100,200,255};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int32
			pd_a3p int32[[1]] b = (int32)(a::uint8);
			int32[[1]] control = {0,100,200,255};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int64
			pd_a3p int[[1]] b = (int)(a::uint8);
			int[[1]] control = {0,100,200,255};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("uint16 -> int");
		bool result = true;
		pd_a3p uint16[[1]] a = {0,15385,38574,65535};
		{
			//int8
			pd_a3p int8[[1]] b = (int8)(a::uint16);
			int8[[1]] control = {0,25,-82,-1};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int16
			pd_a3p int16[[1]] b = (int16)(a::uint16);
			int16[[1]] control = {0,15385,-26962,-1};
			if(any(declassify(b) != control)){result = false;}
			pd_a3p uint16[[1]] temp = a;
			pd_a3p int16[[1]] c = (int16)(temp::uint16);
			if(any(declassify(c) != control)){result = false;}
		}
		{
			//int32
			pd_a3p int32[[1]] b = (int32)(a::uint16);
			int32[[1]] control = {0,15385,38574,65535};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int64
			pd_a3p int[[1]] b = (int)(a::uint16);
			int[[1]] control = {0,15385,38574,65535};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("uint32 -> int");
		bool result = true;
		pd_a3p uint32[[1]] a = {0,21424,21525341,4294967295};
		{
			//int8
			pd_a3p int8[[1]] b = (int8)(a::uint32);
			int8[[1]] control = {0,-80,93,-1};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int16
			pd_a3p int16[[1]] b = (int16)(a::uint32);
			int16[[1]] control = {0,21424,29533,-1};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int32
			pd_a3p int32[[1]] b = (int32)(a::uint32);
			int32[[1]] control = {0,21424,21525341,-1};
			if(any(declassify(b) != control)){result = false;}
			pd_a3p uint32[[1]] temp = a;
			pd_a3p int32[[1]] c = (int32)(temp::uint32);
			if(any(declassify(c) != control)){result = false;}
		}
		{
			//int64
			pd_a3p int[[1]] b = (int)(a::uint32);
			int[[1]] control = {0,21424,21525341,4294967295};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("uint64 -> int");
		bool result = true;
		pd_a3p uint[[1]] a = {0,55161532,142234215413552,18446744073709551615};
		{
			//int8
			pd_a3p int8[[1]] b = (int8)(a::uint);
			int8[[1]] control = {0,-68,48,-1};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int16
			pd_a3p int16[[1]] b = (int16)(a::uint);
			int16[[1]] control = {0,-19780,30512,-1};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int32
			pd_a3p int32[[1]] b = (int32)(a::uint);
			int32[[1]] control = {0, 55161532, 2078439216, -1};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int64
			pd_a3p int[[1]] b = (int)a;
			int[[1]] control = {0,55161532,142234215413552,-1};
			if(any(declassify(b) != control)){result = false;}
			pd_a3p uint[[1]] temp = (a::uint);
			pd_a3p int[[1]] c = (int)(temp::uint);
			if(any(declassify(c) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	print("TEST 5: casting values(4)");
	{
		print("int8 -> uint");
		bool result = true;
		pd_a3p int8[[1]] a = {-128,-40,40,127};
		{
			//int8
			pd_a3p uint8[[1]] b = (uint8) (a::int8);
			uint8[[1]] control = {128,216,40,127};
			if(any(declassify(b) != control)){result = false;}
			pd_a3p int8[[1]] temp = a;
			pd_a3p uint8[[1]] c = (uint8)(temp::int8);
			if(any(declassify(c) != control)){result = false;}
		}
		{
			//int16
			pd_a3p uint16[[1]] b = (uint16)(a::int8);
			uint16[[1]] control = {65408,65496,40,127};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int32
			pd_a3p uint32[[1]] b = (uint32)(a::int8);
			uint32[[1]] control = {4294967168,4294967256,40,127};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int64
			pd_a3p uint[[1]] b = (uint)(a::int8);
			uint[[1]] control = {18446744073709551488,18446744073709551576,40,127};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("int16 -> uint");
		bool result = true;
		pd_a3p int16[[1]] a = {-32768,-16325,12435,32767};
		{
			//int8
			pd_a3p uint8[[1]] b = (uint8)(a::int16);
			uint8[[1]] control = {0,59,147,255};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int16
			pd_a3p uint16[[1]] b = (uint16)(a::int16);
			uint16[[1]] control = {32768,49211,12435,32767};
			if(any(declassify(b) != control)){result = false;}
			pd_a3p int16[[1]] temp = a;
			pd_a3p uint16[[1]] c = (uint16)(temp::int16);
			if(any(declassify(c) != control)){result = false;}
		}
		{
			//int32
			pd_a3p uint32[[1]] b = (uint32)(a::int16);
			uint32[[1]] control = {4294934528, 4294950971,12435,32767};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int64
			pd_a3p uint[[1]] b = (uint)(a::int16);
			uint[[1]] control = {18446744073709518848,18446744073709535291,12435,32767};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("int32 -> uint");
		bool result = true;
		pd_a3p int32[[1]] a = {-2147483648,-483648,2147483,2147483647};
		{
			//int8
			pd_a3p uint8[[1]] b = (uint8)(a::int32);
			uint8[[1]] control = {0,192,155,255};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int16
			pd_a3p uint16[[1]] b = (uint16)(a::int32);
			uint16[[1]] control = {0,40640,50331,65535};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int32
			pd_a3p uint32[[1]] b = (uint32)(a::int32);
			uint32[[1]] control = {2147483648, 4294483648, 2147483, 2147483647};
			if(any(declassify(b) != control)){result = false;}
			pd_a3p int32[[1]] temp = (a::int32);
			pd_a3p uint32[[1]] c = (uint32)(temp::int32);
			if(any(declassify(c) != control)){result = false;}
		}
		{
			//int64
			pd_a3p uint[[1]] b = (uint)(a::int32);
			uint[[1]] control = {18446744071562067968, 18446744073709067968, 2147483, 2147483647};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("int64 -> uint");
		bool result = true;
        pd_a3p int[[1]] a = {-9223372036854775808,-7036854775808,9223372036854,9223372036854775807};
		{
			//int8
			pd_a3p uint8[[1]] b = (uint8)(a::int);
			uint8[[1]] control =  {0, 0, 246, 255};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int16
			pd_a3p uint16[[1]] b = (uint16)(a::int);
			uint16[[1]] control = {0, 20480, 23286, 65535};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int32
			pd_a3p uint32[[1]] b = (uint32)(a::int);
			uint32[[1]] control = {0, 2596622336, 2077252342, 4294967295};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			//int64
			pd_a3p uint[[1]] b = (uint)a;
			uint[[1]] control = {9223372036854775808, 18446737036854775808, 9223372036854, 9223372036854775807};
			if(any(declassify(b) != control)){result = false;}
			pd_a3p int[[1]] temp = (a::int);
			pd_a3p uint[[1]] c = (uint)(temp::int);
			if(any(declassify(c) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}





	print("TEST 6: Casting values(5)");
	{
		bool result = true;
		pd_a3p uint8[[1]] c = {0,100,200,255};
		{
			//uint16
			pd_a3p uint16[[1]] d = (uint16)(c::uint8);
			pd_a3p uint16[[1]] control = {0,100,200,255};
			if(any(declassify(d != control))){result = false;}
		}
		{
			//int32
			pd_a3p uint32[[1]] d = (uint32)(c::uint8);
			pd_a3p uint32[[1]] control = {0,100,200,255};
			if(any(declassify(d != control))){result = false;}
		}
		{
			//int64
			pd_a3p uint[[1]] d = (uint)(c::uint8);
			pd_a3p uint[[1]] control = {0,100,200,255};
			if(any(declassify(d != control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		bool result = true;
		pd_a3p uint16[[1]] c = {0,15385,38574,65535};
		{
			//int8
			pd_a3p uint8[[1]] d = (uint8)(c::uint16);
			pd_a3p uint8[[1]] control = {0, 25, 174, 255};
			if(any(declassify(d != control))){result = false;}
		}
		{
			//int32
			pd_a3p uint32[[1]] d = (uint32)(c::uint16);
			pd_a3p uint32[[1]] control = {0,15385,38574,65535};
			if(any(declassify(d != control))){result = false;}
		}
		{
			//int64
			pd_a3p uint[[1]] d = (uint)(c::uint16);
			pd_a3p uint[[1]] control = {0,15385,38574,65535};
			if(any(declassify(d != control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		bool result = true;
		pd_a3p uint32[[1]] c = {0,21424,21525341,4294967295};
		{
			//int8
			pd_a3p uint8[[1]] d = (uint8)(c::uint32);
			pd_a3p uint8[[1]] control = {0, 176, 93, 255};
			if(any(declassify(d != control))){result = false;}
		}
		{
			//int16
			pd_a3p uint16[[1]] d = (uint16)(c::uint32);
			pd_a3p uint16[[1]] control = {0, 21424, 29533, 65535};
			if(any(declassify(d != control))){result = false;}
		}
		{
			//int64
			pd_a3p uint[[1]] d = (uint)(c::uint32);
			pd_a3p uint[[1]] control = {0,21424,21525341,4294967295};
			if(any(declassify(d != control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		bool result = true;
		pd_a3p uint[[1]] c = {0,55161532,142234215413552,18446744073709551615};
		{
			//int8
			pd_a3p uint8[[1]] d = (uint8)(c::uint);
			pd_a3p uint8[[1]] control = {0, 188, 48, 255};
			if(any(declassify(d != control))){result = false;}
		}
		{
			//int16
			pd_a3p uint16[[1]] d = (uint16)(c::uint);
			pd_a3p uint16[[1]] control = {0, 45756, 30512, 65535};
			if(any(declassify(d != control))){result = false;}
		}
		{
			//int32
			pd_a3p uint32[[1]] d = (uint32)(c::uint);
			pd_a3p uint32[[1]] control = {0, 55161532, 2078439216, 4294967295};
			if(any(declassify(d != control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}






	{
		bool result = true;
		pd_a3p int8[[1]] c = {-128,-40,40,127};
		{
			//int16
			pd_a3p int16[[1]] b = (int16)(c::int8);
			pd_a3p int16[[1]] control = {-128,-40,40,127};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			//int32
			pd_a3p int32[[1]] b = (int32)(c::int8);
			pd_a3p int32[[1]] control = {-128,-40,40,127};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			//int64
			pd_a3p int[[1]] b = (int)(c::int8);
			pd_a3p int[[1]] control = {-128,-40,40,127};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		bool result = true;
		pd_a3p int16[[1]] c = {-32768,-16325,12435,32767};
		{
			//int8
			pd_a3p int8[[1]] b = (int8)(c::int16);
			pd_a3p int8[[1]] control = {0, 59, -109, -1};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			//int32
			pd_a3p int32[[1]] b = (int32)(c::int16);
			pd_a3p int32[[1]] control = {-32768,-16325,12435,32767};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			//int64
			pd_a3p int[[1]] b = (int)(c::int16);
			pd_a3p int[[1]] control = {-32768,-16325,12435,32767};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		bool result = true;
		pd_a3p int32[[1]] c = {-2147483648,-483648,2147483,2147483647};
		{
			//int8
			pd_a3p int8[[1]] b = (int8)(c::int32);
			pd_a3p int8[[1]] control = {0, -64, -101, -1};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			//int16
			pd_a3p int16[[1]] b = (int16)(c::int32);
			pd_a3p int16[[1]] control =  {0, -24896, -15205, -1};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			//int64
			pd_a3p int[[1]] b = (int)(c::int32);
			pd_a3p int[[1]] control = {-2147483648,-483648,2147483,2147483647};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		bool result = true;
		pd_a3p int[[1]] c = {-9223372036854775808,-7036854775808,9223372036854,9223372036854775807};
		{
			//int8
			pd_a3p int8[[1]] b = (int8)(c::int);
			pd_a3p int8[[1]] control =  {0, 0, -10, -1};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			//int16
			pd_a3p int16[[1]] b = (int16)(c::int);
			pd_a3p int16[[1]] control = {0, 20480, 23286, -1};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			//int32
			pd_a3p int32[[1]] b = (int32)(c::int);
			pd_a3p int32[[1]] control = {0, -1698344960, 2077252342, -1};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}




	print("TEST 7: Casting values(6)");
	{
		print("uint8 -> float32/64");
		bool result = true;
		pd_a3p uint8[[1]] a = {0,100,200,255};
		{
			pd_a3p float32[[1]] b = (float32)(a::uint8);
			float32[[1]] control = {0,100,200,255};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			pd_a3p float64[[1]] b = (float64)(a::uint8);
			float64[[1]] control = {0,100,200,255};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("uint16 -> float32/64");
		bool result = true;
		pd_a3p uint16[[1]] a = {0,15385,38574,65535};
		{
			pd_a3p float32[[1]] b = (float32)(a::uint16);
			float32[[1]] control = {0,15385,38574,65535};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			pd_a3p float64[[1]] b = (float64)(a::uint16);
			float64[[1]] control = {0,15385,38574,65535};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
			print("FAILURE! casting failed");
			all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("uint32 -> float32/64");
		bool result = true;
		pd_a3p uint32[[1]] a = {0,21424,21525341,4294967295};
		{
			pd_a3p float32[[1]] b = (float32)(a::uint32);
			float32[[1]] control = {0,21424,21525341,4294967295};
			if(any(declassify(b) != control)){result = false;}
		}
		{
			pd_a3p float64[[1]] b = (float64)(a::uint32);
			float64[[1]] control = {0,21424,21525341,4294967295};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("uint64 -> float64");
		bool result = true;
		pd_a3p uint[[1]] a = {0,55161532,142234215413552,18446744073709551615};
		{
			pd_a3p float64[[1]] b = (float64)(a::uint);
			float64[[1]] control = {0,55161532,142234215413552,18446744073709551615};
			if(any(declassify(b) != control)){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("int8 -> Float32/64");
        bool result = true;
		pd_a3p int8[[1]] c = {-128,-40,40,127};
		{
			pd_a3p float32[[1]] b = (float32)(c::int8);
			pd_a3p float32[[1]] control = {-128,-40,40,127};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			pd_a3p float64[[1]] b = (float64)(c::int8);
			pd_a3p float64[[1]] control = {-128,-40,40,127};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("int16 -> Float32/64");
		bool result = true;
		pd_a3p int16[[1]] c = {-32768,-16325,12435,32767};
		{
			pd_a3p float32[[1]] b = (float32)(c::int16);
			pd_a3p float32[[1]] control = {-32768,-16325,12435,32767};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		{
			pd_a3p float64[[1]] b = (float64)(c::int16);
			pd_a3p float64[[1]] control = {-32768,-16325,12435,32767};
			if(any(declassify(b) != declassify(control))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("int32 -> Float32/64");
		bool result = true;
		pd_a3p int32[[1]] c = {-2147483648,-483648,2147483,2147483647};
		{
			pd_a3p float32[[1]] b = (float32)(c::int32);
			pd_a3p float32[[1]] control = {-2147483648,-483648,2147483,2147483647};
			if(!all(declassify(isNegligible(control - b)))){result = false;}
		}
		{
			pd_a3p float64[[1]] b = (float64)(c::int32);
			pd_a3p float64[[1]] control =  {-2147483648,-483648,2147483,2147483647};
			if(!all(declassify(isNegligible(control - b)))){result = false;}
		}
		if(!result){
		print("FAILURE! casting failed");
		all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}
	{
		print("int64 -> Float32/64");
		bool result = true;
		pd_a3p int[[1]] c = {-9223372036854775808,-7036854775808,9223372036854,9223372036854775807};
		{
			pd_a3p float32[[1]] b = (float32)(c::int);
			pd_a3p float32[[1]] control = {-9223372036854775808,-7036854775808,9223372036854,9223372036854775807};
			if(!all(declassify(isNegligible(control - b)))){result = false;}
		}
		{
			pd_a3p float64[[1]] b = (float64)(c::int);
			pd_a3p float64[[1]] control = {-9223372036854775808,-7036854775808,9223372036854,9223372036854775807};
			if(!all(declassify(isNegligible(control - b)))){result = false;}
		}
		if(!result){
            print("FAILURE! casting failed");
            all_tests += 1;
		}
		else{
			print("SUCCESS!");
			succeeded_tests +=1;
			all_tests += 1;
		}
	}


	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);

    test_report(all_tests, succeeded_tests);
}
