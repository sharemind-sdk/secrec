/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module float_erf;

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
void test_erf(T data){
	T percentage;
	pd_a3p T[[1]] a (12) = {
		-1,
		-0.8,
		-0.6,
		-0.4,
		-0.2,
		-0.1,
		0.1,
		0.2,
		0.4,
		0.6,
		0.8,
		1
	};


	T[[1]] b (12) = {
		-0.84270079294971486934122063508260925929606699796630290845993,
		-0.74210096470766048616711058650294587731768957991470872688213,
		-0.60385609084792592256262243605672320656427336480009790555297,
		-0.42839235504666845510360384532017244412186292852259038349508,
		-0.22270258921047845414013900680014381638826903843022760562093,
		-0.11246291601828489220327507174396838322169629915970254753449,
		0.112462916018284892203275071743968383221696299159702547534494,
		0.222702589210478454140139006800143816388269038430227605620935,
		0.428392355046668455103603845320172444121862928522590383495086,
		0.603856090847925922562622436056723206564273364800097905552970,
		0.742100964707660486167110586502945877317689579914708726882135,
		0.842700792949714869341220635082609259296066997966302908459937
	};

	pd_a3p T[[1]] c (12);

	c = erf(a);
	T[[1]] d (12);
	T[[1]] temp(12) = declassify(a);
	
	d = declassify(c) - b;

	for(uint i = 0; i < 10;++i){
		print("Erf(",temp[i],") = ",declassify(c[i])," Expected: ",b[i]);
		if(d[i] < 0){d[i] = -d[i];}
		print("absolute difference: ",d[i]);
		print("---------------------------");

		if(temp[i] < 0){temp[i] = -temp[i];}
	}
	percentage = sum(d / temp); 
	print("TEST completed");
	print("Relative imprecision of Erf is: ", ((percentage / 12)*100), " %");
}


void main(){
	print("Erf test: start");
	{
		print("Float32");
		test_erf(0::float32);
	}
	{
		print("Float64");
		test_erf(0::float64);
	}
}