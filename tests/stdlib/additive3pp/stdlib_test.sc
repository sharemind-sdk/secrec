/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module stdlib_test;

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
void equal_shapes_test(T data){
	{
		pd_a3p T[[2]] mat (0,0);
		pd_a3p T[[2]] mat2 (0,0);
		bool result = shapesAreEqual(mat,mat2);
		pd_a3p T[[2]] mat3 (0,2);
		pd_a3p T[[2]] mat4 (0,2);
		result = shapesAreEqual(mat3,mat4);
		pd_a3p T[[2]] mat5 (2,0);
		pd_a3p T[[2]] mat6 (2,0);
		result = shapesAreEqual(mat5,mat6);
	}
	{
		pd_a3p T[[2]] mat (5,5);
		pd_a3p T[[2]] mat2 (5,5);
		T[[2]] mat3 = declassify(randomize(mat));
		T[[2]] mat4 = declassify(randomize(mat2));
		bool result = shapesAreEqual(mat3,mat4);
		if(all(shape(mat3) == shape(mat4))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			all_tests = all_tests +1;
			print("FAILURE! Shapes should be equal but they're not");
		}
	}
	{
		pd_a3p T[[2]] mat (4,6);
		pd_a3p T[[2]] mat2 (24,3);
		T[[2]] mat3 = declassify(randomize(mat));
		T[[2]] mat4 = declassify(randomize(mat2));
		bool result = shapesAreEqual(mat3,mat4);
		if(!all(shape(mat3) == shape(mat4))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
		}
		else{
			all_tests = all_tests +1;
			print("FAILURE! Shapes should not be equal but they are");
		}
	}
}

void main(){
	print("Standard library test: start");

	print("TEST 2: Shapes are equal utility");
	{
		print("bool");
		equal_shapes_test(false);
	}
	{
		print("uint8");
		equal_shapes_test(0::uint8);
	}
	{
		print("uint16");
		equal_shapes_test(0::uint16);
	}
	{
		print("uint32");
		equal_shapes_test(0::uint32);
	}
	{
		print("uint64/uint");
		equal_shapes_test(0::uint);
	}
	{
		print("int8");
		equal_shapes_test(0::int8);
	}
	{
		print("int16");
		equal_shapes_test(0::int16);
	}
	{
		print("int32");
		equal_shapes_test(0::int32);
	}
	{
		print("int64/int");
		equal_shapes_test(0::int);
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);

    test_report(all_tests, succeeded_tests);
}
