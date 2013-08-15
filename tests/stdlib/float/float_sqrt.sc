/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module float_sqrt;

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


template<type T>
void test_sqrt(T data){
	T percentage;
	pd_a3p T[[1]] a (20) = {
		0.1,
		0.2,
		0.3,
		0.4,
		0.5,
		0.6,
		0.7,
		0.8,
		0.9,
		1,
		2,
		3,
		4,
		5,
		10,
		50,
		100,
		500,
		1000,
		10000
	};


	T[[1]] b (20) = {
		0.316227766016837933199889354443271853371955513932521682685750,
		0.447213595499957939281834733746255247088123671922305144854179,
		0.547722557505166113456969782800802133952744694997983254226894,
		0.632455532033675866399778708886543706743911027865043365371500,
		0.707106781186547524400844362104849039284835937688474036588339,
		0.774596669241483377035853079956479922166584341058318165317514,
		0.836660026534075547978172025785187489392815369298672199811191,
		0.894427190999915878563669467492510494176247343844610289708358,
		0.948683298050513799599668063329815560115866541797565048057251,
		1,
		1.414213562373095048801688724209698078569671875376948073176679,
		1.732050807568877293527446341505872366942805253810380628055806,
		2,
		2.236067977499789696409173668731276235440618359611525724270897,
		3.162277660168379331998893544432718533719555139325216826857504,
		7.071067811865475244008443621048490392848359376884740365883398,
		10,
		22.36067977499789696409173668731276235440618359611525724270897,
		31.62277660168379331998893544432718533719555139325216826857504,
		100

	};

	pd_a3p T[[1]] c (20);

	c = sqrt(a);
	T[[1]] d (20);
	T[[1]] temp(20) = declassify(a);
	
	d = declassify(c) - b;

	for(uint i = 0; i < 20;++i){
		print("Sqrt(",temp[i],") = ",declassify(c[i])," Expected: ",b[i]);
		if(d[i] < 0){d[i] = -d[i];}
		print("absolute difference: ",d[i]);
		print("---------------------------");

		if(temp[i] < 0){temp[i] = -temp[i];}
	}
	percentage = sum(d / temp); 
	print("TEST completed");
	print("Relative imprecision of Square root is: ", ((percentage / 20)*100), " %");
}


void main(){
	print("Sqrt test: start");
	{
		print("Float32");
		test_sqrt(0::float32);
	}
	{
		print("Float64");
		test_sqrt(0::float64);
	}
}