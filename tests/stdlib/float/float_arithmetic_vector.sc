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

domain pd_a3p additive3pp;

public uint32 all_tests;
public uint32 succeeded_tests;

void Success(){
	succeeded_tests = succeeded_tests + 1;
	all_tests = all_tests +1;
	print("SUCCESS!");
}
template<type T,domain D: additive3pp>
void Failure(string s, D T[[1]] c){
	print("FAILURE! ",s);
	print("got: ");
	printVector(declassify(c));
	all_tests = all_tests +1;
}
template<type T>
void Failure(string s,T[[1]] c){
	print("FAILURE! ",s);
	print("got: ");
	printVector(c);
	all_tests = all_tests +1;
}

void main(){

	print("Arithmetic test: start");

	print("TEST 1: Addition with two public vectors");
	{
		print("float32");
		float32[[1]] a (15) = 15; float32[[1]] b (15)= 174 ; float32[[1]] c = a+b;
		if(all(c == 189)){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("float64");
		float64[[1]] a (15) = 175; float64[[1]] b (15)= 45876; float64[[1]] c = a+b;
		if(all(c == 46051)){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 2: Addition with two private vectors");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; pd_a3p float32[[1]] b (15)= 174; pd_a3p float32[[1]] c = a+b;
		if(all(declassify(c) == 189)){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 175; pd_a3p float64[[1]] b (15)= 45876; pd_a3p float64[[1]] c = a+b;
		if(all(declassify(c) == 46051)){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 3: Addition with one private one public vector");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; float32[[1]] b (15)= 174; pd_a3p float32[[1]] c = a+b;
		if(all(declassify(c) == 189)){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 175; float64[[1]] b (15)= 45876; pd_a3p float64[[1]] c = a+b;
		if(all(declassify(c) == 46051)){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 4: Addition with one private one public vector(2)");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; float32[[1]] b (15)= 174; pd_a3p float32[[1]] c = b+a;
		if(all(declassify(c) == 189)){Success();}else{Failure("Expected 189",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 175; float64[[1]] b (15)= 45876; pd_a3p float64[[1]] c = b+a;
		if(all(declassify(c) == 46051)){Success();}else{Failure("Expected 46051",c);}
	}
















	print("TEST 5: Subtraction with two public vectors");
	{
		print("float32");
		float32[[1]] a (15)= 15; float32[[1]] b (15)= 174; float32[[1]] c = b-a;
		if(all(c == 159)){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("float64");
		float64[[1]] a (15)= 175; float64[[1]] b (15)= 45876; float64[[1]] c = b-a;
		if(all(c == 45701)){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 6: Subtraction with two private vectors");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; pd_a3p float32[[1]] b (15)= 174; pd_a3p float32[[1]] c = b-a;
		if(all(declassify(c) == 159)){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 175; pd_a3p float64[[1]] b (15)= 45876; pd_a3p float64[[1]] c = b-a;
		if(all(declassify(c) == 45701)){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 7: Subtraction with one private one public vector");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; float32[[1]] b (15)= 174; pd_a3p float32[[1]] c = b-a;
		if(all(declassify(c) == 159)){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 175; float64[[1]] b (15)= 45876; pd_a3p float64[[1]] c = b-a;
		if(all(declassify(c) == 45701)){Success();}else{Failure("Expected 46051",c);}
	}
	print("TEST 8: Subtraction with one private one public value(2)");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 174; float32[[1]] b (15)= 15; pd_a3p float32[[1]] c = a-b;
		if(all(declassify(c) == 159)){Success();}else{Failure("Expected 159",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 45876; float64[[1]] b (15)= 175; pd_a3p float64[[1]] c = a-b;
		if(all(declassify(c) == 45701)){Success();}else{Failure("Expected 45701",c);}
	}

















	print("TEST 9: Multiplication with two public vectors");
	{
		print("float32");
		float32[[1]] a (15)= 15; float32[[1]] b (15)= 12; float32[[1]] c = a*b;
		if(all(c == 180)){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("float64");
		float64[[1]] a (15)= 175; float64[[1]] b (15)= 139; float64[[1]] c = a*b;
		if(all(c == 24325)){Success();}else{Failure("Expected 24325",c);}
	}
	print("TEST 10: Multiplication with two private vectors");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; pd_a3p float32[[1]] b (15)= 12; pd_a3p float32[[1]] c = a*b;
		if(all(declassify(c) == 180)){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 175; pd_a3p float64[[1]] b (15)= 139; pd_a3p float64[[1]] c = a*b;
		if(all(declassify(c) == 24325)){Success();}else{Failure("Expected 24325",c);}
	}
	print("TEST 11: Multiplication with one private one public vector");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; float32[[1]] b (15)= 12; pd_a3p float32[[1]] c = a*b;
		if(all(declassify(c) == 180)){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 175; float64[[1]] b (15)= 139; pd_a3p float64[[1]] c = a*b;
		if(all(declassify(c) == 24325)){Success();}else{Failure("Expected 24325",c);}
	}
	print("TEST 12: Multiplication with one private one public value(2)");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; float32[[1]] b (15)= 12; pd_a3p float32[[1]] c = b*a;
		if(all(declassify(c) == 180)){Success();}else{Failure("Expected 180",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 175; float64[[1]] b (15)= 139; pd_a3p float64[[1]] c = b*a;
		if(all(declassify(c) == 24325)){Success();}else{Failure("Expected 24325",c);}
	}

















	print("TEST 13: Division with two public vectors");
	{
		print("float32");
		float32[[1]] a (15)= 15; float32[[1]] b (15)= 174; float32[[1]] c = b/a;
		if(all(c == 11.6)){Success();}else{Failure("Expected 11.6",c);}
	}
	{
		print("float64");
		float64[[1]] a (15)= 180; float64[[1]] b (15)= 45900; float64[[1]] c = b/a;
		if(all(c == 255)){Success();}else{Failure("Expected 255",c);}
	}
	print("TEST 14: Division with two private values");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; pd_a3p float32[[1]] b (15)= 174; pd_a3p float32[[1]] c = b/a;
		if(all(declassify(c) == 11.6)){Success();}else{Failure("Expected 11.6",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 180; pd_a3p float64[[1]] b (15)= 45900; pd_a3p float64[[1]] c = b/a;
		if(all(declassify(c) == 255)){Success();}else{Failure("Expected 255",c);}
	}
	print("TEST 15: Division with one private one public value");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; float32[[1]] b (15)= 174; pd_a3p float32[[1]] c = b/a;
		if(all(declassify(c) == 11.6)){Success();}else{Failure("Expected 11.6",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 180; float64[[1]] b (15)= 45900; pd_a3p float64[[1]] c = b/a;
		if(all(declassify(c) == 255)){Success();}else{Failure("Expected 255",c);}
	}
	print("TEST 16: Division with one private one public value(2)");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 174; float32[[1]] b (15)= 15; pd_a3p float32[[1]] c = a/b;
		if(all(declassify(c) == 11.6)){Success();}else{Failure("Expected 11.6",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 45900; float64[[1]] b (15)= 180; pd_a3p float64[[1]] c = a/b;
		if(all(declassify(c) == 255)){Success();}else{Failure("Expected 255",c);}
	}
	print("TEST 17: 0 divided with random public vectors");
	{
		print("float32");
		float32[[1]] a (15)= 15; float32[[1]] b (15)= 0; float32[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("float64");
		float64[[1]] a (15)= 180; float64[[1]] b (15)= 0; float64[[1]] c = b/a;
		if(all(c == 0)){Success();}else{Failure("Expected 0",c);}
	}
	print("TEST 18: 0 divided with random private vectors");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 15; float32[[1]] b (15)= 0; pd_a3p float32[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 180; float64[[1]] b (15)= 0; pd_a3p float64[[1]] c = b/a;
		if(all(declassify(c) == 0)){Success();}else{Failure("Expected 0",c);}
	}
	print("TEST 19: A/A = 1 with all public types");
	{
		print("float32");
		float32[[1]] a (15)= 174; float32[[1]] b (15)= 174; float32[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1",c);}
	}
	{
		print("float64");
		float64[[1]] a (15)= 45876; float64[[1]] b (15)= 45876; float64[[1]] c = b/a;
		if(all(c == 1)){Success();}else{Failure("Expected 1",c);}
	}
	print("TEST 20: Division accuracy public");
	{
		print("float32");
		float32[[1]] a (15)= 645; float32[[1]] b (15)= 40; float32[[1]] c = a/b;
		if(all(c == 16.125)){Success();}else{Failure("Expected 16.125",c);}
	}
	{
		print("float64");
		float64[[1]] a (15)= 645; float64[[1]] b (15)= 40; float64[[1]] c = a/b;
		if(all(c == 16.125)){Success();}else{Failure("Expected 16.125",c);}
	}
	print("TEST 21: Division accuracy private");
	{
		print("float32");
		pd_a3p float32[[1]] a (15)= 645; pd_a3p float32[[1]] b (15)= 40; pd_a3p float32[[1]] c = a/b;
		if((all(declassify(c) == 16.125))){Success();}else{Failure("Expected 16.125",c);}
	}
	{
		print("float64");
		pd_a3p float64[[1]] a (15)= 645; pd_a3p float64[[1]] b (15)= 40; pd_a3p float64[[1]] c = a/b;
		if((all(declassify(c) == 16.125))){Success();}else{Failure("Expected 16.125",c);}
	}

















	print("TEST 22: Operation priorities : Multiplication over addition");
	{
		print("float32");
		float32[[1]] a (15)= 5; float32[[1]] b (15)= 20; float32[[1]] c (15)= 45;
		float32[[1]] d (15)= c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145",d);}
	}
	{
		print("float64");
		float64[[1]] a (15)= 5; float64[[1]] b (15)= 20; float64[[1]] c (15)= 45;
		float64[[1]] d = c + b * a;
		if(all(d == 145)){Success();}else{Failure("Expected 145",d);}
	}
	print("TEST 23: Operation priorities : Parentheses over multiplication");
	{
		print("float32");
		float32[[1]] a (15)= 5; float32[[1]] b (15)= 5; float32[[1]] c (15)= 20;
		float32[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125",d);}
	}
	{
		print("float64");
		float64[[1]] a (15)= 5; float64[[1]] b (15)= 5; float64[[1]] c (15)= 20;
		float64[[1]] d = (c + b) * a;
		if(all(d == 125)){Success();}else{Failure("Expected 125",d);}
	}
	print("TEST 24: Operation priorities : Division over addition and subtraction");
	{
		print("float32");
		float32[[1]] a (15)= 5; float32[[1]] b (15)= 5; float32[[1]] c (15)= 20; float32[[1]] d (15)= 5;
		float32[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16",e);}
	}
	{
		print("float64");
		float64[[1]] a (15)= 5; float64[[1]] b (15)= 5; float64[[1]] c (15)= 20; float64[[1]] d (15)= 5;
		float64[[1]] e = c - a + b / d;
		if(all(e == 16)){Success();}else{Failure("Expected 16",e);}
	}
	print("TEST 38: public boolean negotiation (!)");
	{
		print("float32");
		float32[[1]] a (15)= 25;
		float32[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negotiation failed",(a != b));}
	}
	{
		print("float64");
		float64[[1]] a (15)= 25;
		float64[[1]] b (15)= 26;
		if(all(a != b)){Success();}
		else{Failure("Boolean negotiation failed",(a != b));}
	}



	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}