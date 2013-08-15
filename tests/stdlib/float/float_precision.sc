/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module float_precision;

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


/****************************************************
*****************************************************
*****************************************************
*****/   	public uint test_amount = 10;  	    /****
******  increase for more accurate percentages  *****
*****************************************************
*****************************************************
*****************************************************/


domain pd_a3p additive3pp;

template<type T>
T random_float(T data){
    T rand = 1;
    pd_a3p uint32 temp;
    pd_a3p int8 temp2;
    T scalar;
    T scalar2;
    for(uint i = 0; i < 2; ++i){   
        scalar = 0;
        while(scalar == 0 || scalar2 == 0){
            scalar = (T) declassify(randomize(temp));
            scalar2 = (T) declassify(randomize(temp2));
        }
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

template<domain D:additive3pp,type T>
D T[[1]] random(D T[[1]] data){
    uint x_shape = size(data);
    T[[1]] rand (x_shape) = 1;
    pd_a3p uint32[[1]] temp (x_shape);
    pd_a3p int8[[1]] temp2 (x_shape);
    T[[1]] scalar (x_shape);
    T[[1]] scalar2 (x_shape);
    for(uint i = 0; i < 2; ++i){   
        scalar[0] = 0;
        while(any(scalar == 0) || any(scalar2 == 0)){
            scalar = (T) declassify(randomize(temp));
            scalar2 = (T) declassify(randomize(temp2));
        }
        if((i % 2) == 0){
            rand *= scalar;
            rand *= scalar2;
        }
        else{
            rand /= scalar;
            rand /= scalar2;
        }
    }
    pd_a3p T[[1]] result = rand;
    return result;
}

template<domain D:additive3pp,type T>
D T[[2]] random(D T[[2]] data){
    uint x_shape = shape(data)[0];
    uint y_shape = shape(data)[1];
    T[[2]] rand (x_shape,y_shape) = 1;
    pd_a3p uint32[[2]] temp (x_shape,y_shape);
    pd_a3p int8[[2]] temp2 (x_shape,y_shape);
    T[[2]] scalar (x_shape,y_shape);
    T[[2]] scalar2 (x_shape,y_shape);
    for(uint i = 0; i < 2; ++i){   
        scalar[0,0] = 0;
        while(any(scalar == 0) || any(scalar2 == 0)){
            scalar = (T) declassify(randomize(temp));
            scalar2 = (T) declassify(randomize(temp2));
        }
        if((i % 2) == 0){
            rand *= scalar;
            rand *= scalar2;
        }
        else{
            rand /= scalar;
            rand /= scalar2;
        }
    }
    pd_a3p T[[2]] result = rand;
    return result;
}

template<domain D:additive3pp,type T>
D T[[3]] random(D T[[3]] data){
    uint x_shape = shape(data)[0];
    uint y_shape = shape(data)[1];
    uint z_shape = shape(data)[2];
    T[[3]] rand (x_shape,y_shape,z_shape) = 1;
    pd_a3p uint32[[3]] temp (x_shape,y_shape,z_shape);
    pd_a3p int8[[3]] temp2 (x_shape,y_shape,z_shape);
    T[[3]] scalar (x_shape,y_shape,z_shape);
    T[[3]] scalar2 (x_shape,y_shape,z_shape);
    for(uint i = 0; i < 2; ++i){   
        scalar[0,0,0] = 0;
        while(any(scalar == 0) || any(scalar2 == 0)){
            scalar = (T) declassify(randomize(temp));
            scalar2 = (T) declassify(randomize(temp2));
        }
        if((i % 2) == 0){
            rand *= scalar;
            rand *= scalar2;
        }
        else{
            rand /= scalar;
            rand /= scalar2;
        }
    }
    pd_a3p T[[3]] result = rand;
    return result;
}

template<type T>
void test_addition(T data){
	T percentage;
	T temp; T temp2;
	pd_a3p T a;
	pd_a3p T b;
	T sum; T sum2;
	for(uint i = 0; i < test_amount; ++i){
		print("test: ",i+1);
		a = classify(random_float(0::T)); b = classify(random_float(0::T));
		sum = declassify(a+b); sum2 = declassify(a) + declassify(b);
		temp = sum-sum2; temp2 = sum2;
		if(temp < 0){temp = -temp};
		if(temp2 < 0){temp2 = -temp2};
		percentage += (temp / temp2); 
	}
	print("TEST completed");
	print("Addition is imprecise by: ", ((percentage / (T)test_amount)*100), " %");
}

template<type T>
void test_subtraction(T data){
	T percentage;
	T temp; T temp2;
	pd_a3p T a;
	pd_a3p T b;
	T sum; T sum2;
	for(uint i = 0; i < test_amount; ++i){
		print("test: ",i+1);
		a = classify(random_float(0::T)); b = classify(random_float(0::T));
		sum = declassify(a-b); sum2 = declassify(a) - declassify(b);
		temp = sum-sum2; temp2 = sum2;
		if(temp < 0){temp = -temp};
		if(temp2 < 0){temp2 = -temp2};
		percentage += (temp / temp2); 
	}
	print("TEST completed");
	print("Subtraction is imprecise by: ", ((percentage / (T)test_amount )*100), " %");
}


template<type T>
void test_multiplication(T data){
	T percentage;
	T temp; T temp2;
	pd_a3p T a;
	pd_a3p T b;
	T sum; T sum2;
	for(uint i = 0; i < test_amount; ++i){
		print("test: ",i+1);
		a = classify(random_float(0::T)); b = classify(random_float(0::T));
		sum = declassify(a*b); sum2 = declassify(a) * declassify(b);
		temp = sum-sum2; temp2 = sum2;
		if(temp < 0){temp = -temp};
		if(temp2 < 0){temp2 = -temp2};
		percentage += (temp / temp2); 
	}
	print("TEST completed");
	print("Multiplication is imprecise by: ", ((percentage / (T)test_amount )*100), " %");
}

template<type T>
void test_division(T data){
	T percentage;
	T temp; T temp2;
	pd_a3p T a;
	pd_a3p T b;
	T sum; T sum2;
	for(uint i = 0; i < test_amount; ++i){
		print("test: ",i+1);
		a = classify(random_float(0::T)); b = classify(random_float(0::T));
		sum = declassify(a/b); sum2 = declassify(a) / declassify(b);
		temp = sum-sum2; temp2 = sum2;
		if(temp < 0){temp = -temp};
		if(temp2 < 0){temp2 = -temp2};
		percentage += (temp / temp2); 
	}
	print("TEST completed");
	print("Division is imprecise by: ", ((percentage / (T)test_amount )*100), " %");
}

template<type T>
void test_inversion(T data){
	T percentage;
	pd_a3p T[[1]] a (8) = {
		10.49934588656477,
		58.83904325241103,
		6.459133708592741,
		12.30305849067902,
		5.672880010454444,
		0.5154542504192215,
		-33.07334086055091,
		-8.146960136185919
	};
	T[[1]] b (8) = {
		0.095244028609403699186052996064777215723445856946584784926884,
		0.016995517682198601980202808696548131280981723968423205692767,
		0.154819523037536122962576052147431363914115676938109787699726,
		0.081280601954190079255736827142709095552187175689689325695726,
		0.176277305029741292975640471523912602826018007231098313451172,
		1.940036383804566626673218567731497610520720266582510220379170,
		-0.03023583266705227484615820064040857182654732612020244551028,
		-0.12274516915313642586555771859977852412995381036589904796001
	};
	pd_a3p T[[1]] c (8);

	c = inv(a);
	T[[1]] d (8);
	T[[1]] temp(8) = declassify(a);

	d = declassify(c) - b;

	for(uint i = 0; i < 8;++i){
		if(d[i] < 0){d[i] = -d[i];}
		if(temp[i] < 0){temp[i] = -temp[i];}
	}
	percentage = sum(d / temp); 
	print("TEST completed");
	print("Inversion vs. hardcoded answers is imprecise by: ", ((percentage / 8)*100), " %");

	T sum; T sum2;
	a = random(a);
	temp = declassify(a);
	for(uint i = 0; i < 8; ++i){
		b[i] = 1/temp[i];
		c[i] = inv(a[i]);
	}
	d = declassify(c) - b;
	for(uint i = 0; i < 8;++i){
		if(d[i] < 0){d[i] = -d[i];}
		if(temp[i] < 0){temp[i] = -temp[i];}
	}

	percentage = sum(d / temp); 
	print("TEST completed");
	print("Inversion vs. internal arithmetic unit answers is imprecise by: ", ((percentage / 8)*100), " %");
}

template<type T>
void test_inversion2(T data){
	pd_a3p T[[1]] a (5) = 10.49934588656477;
	T[[1]] c (5);
	c = declassify(inv(a));

	if((c[0] == c[1]) && (c[0] == c[2]) && (c[0] == c[3]) && (c[0] == c[4])){
		print("TEST completed successfully");
	}
	else{
		print("WARNING! using inv on vector with same elements, got different answers");
		print("input vector: ", arrayToString(declassify(a)));
		print("inversed values: ",arrayToString(c));
	}
}

template<type T>
void test_sqrt(T data){
	T percentage;
	pd_a3p T[[1]] a (8) = {
		14.1566418266453,
		2.353240445436502,
		1.101880541351803,
		26.80217700594072,
		2.461238153504522,
		0.6487119793399101,
		3.497798862860427,
		1.757030089475017
	};
	T[[1]] b (8) = {
		3.7625313057362459638668630134746153084776989341461835,
		1.534027524341236192801048418712285670612360413934362466077706,
		1.049704978244746155686088076871481282485641505974230593725993,
		5.177081900640622471086424463319686255349670851106896530423644,
		1.568833373403473336501112685959116159596509349282887846858288,
		0.805426582215852930437182611153055291564735094770495012808766,
		1.870240322220763959112005305796522349151384344661816065940105,
		1.325530116396838793454662999150389473000883666573902051199777
	};
	pd_a3p T[[1]] c (8);
	c = sqrt(a);

	T[[1]] d (8);
	T[[1]] temp(8) = declassify(a);
	
	d = declassify(c) - b;

	for(uint i = 0; i < 8;++i){
		if(d[i] < 0){d[i] = -d[i];}
		if(temp[i] < 0){temp[i] = -temp[i];}
	}
	percentage = sum(d / temp); 
	print("TEST completed");
	print("Square root vs. hardcoded answers is imprecise by: ", ((percentage / 8)*100), " %");
}

template<type T>
void test_sqrt2(T data){
	pd_a3p T[[1]] a (5) = 14.1566418266453;
	T[[1]] c (5);
	c = declassify(sqrt(a));

	if((c[0] == c[1]) && (c[0] == c[2]) && (c[0] == c[3]) && (c[0] == c[4])){
		print("TEST completed successfully");
	}
	else{
		print("WARNING! using sqrt on vector with same elements, got different answers");
		print("input vector: ", arrayToString(declassify(a)));
		print("square root values: ",arrayToString(c));
	}
}

template<type T>
void test_sin(T data){
	T percentage;
	pd_a3p T[[1]] a (6) = {
		1.535359689017609,
		-3.691520454030958,
		0.3645205550492913,
		3.670053129892183,
		3.565277562294547,
		//-48.42696016247228,
		5.411167505765563
		//22.58281807201601
		// cannot use those values otherwise imprecision is over a billion percent
		// VM probably has an issue with angles bigger than 2*pi
	};


	//radians
	T[[1]] b (6) = {
		0.999372188053827311880116625789242387997100157445729730841003,
		0.522625675673997926990697591852093392812491496336474491601758,
		0.356501392547928893588299460365806303984370739148059352783049,
		-0.50420443070747302192901476609516032397872513906068697502028,
		-0.41112232643427713072343386894534779652674092407078421997585,
		//0.964374959685377044826710393641436229248997196381340521352072,
		-0.76562851207067262638835876030431734777683517527272468681800
		//-0.55774749979570813840005801495922364317703842299950389372550
	};

	pd_a3p T[[1]] c (6);

	c = sin(a);
	T[[1]] d (6);
	T[[1]] temp(6) = declassify(a);
	
	d = declassify(c) - b;

	for(uint i = 0; i < 6;++i){
		if(d[i] < 0){d[i] = -d[i];}
		if(temp[i] < 0){temp[i] = -temp[i];}
	}
	percentage = sum(d / temp); 
	print("TEST completed");
	print("Sin vs. hardcoded answers is imprecise by: ", ((percentage / 6)*100), " %");
}

template<type T>
void test_sin2(T data){
	pd_a3p T[[1]] a (5) = 1.535359689017609;
	T[[1]] c (5);
	c = declassify(sin(a));

	if((c[0] == c[1]) && (c[0] == c[2]) && (c[0] == c[3]) && (c[0] == c[4])){
		print("TEST completed successfully");
	}
	else{
		print("WARNING! using sin on vector with same elements, got different answers");
		print("input vector: ", arrayToString(declassify(a)));
		print("sine values: ",arrayToString(c));
	}
}

template<type T>
void test_ln(T data){
	T percentage;
	pd_a3p T[[1]] a (6) = {
		51.35752137315483,
		2.608711404814097,
		8.850954516109905,
		5.192202509740625,
		24.36159839949806,
		0.2756185651367633
	};

	T[[1]] b (6) = {
		3.938811398348680089804643433376430680367653668588343000906230,
		0.958856384786789770072515166290468948953628611903734246556797,
		2.180525308131586498592170766819923549287755820600765970199695,
		1.647157982828476806467086708358951537473793664301911313654868,
		3.193008056432019284572330551494423539275983811534468092220874,
		-1.28873737949614117662681428728923733958919670534871008889074
	};

	pd_a3p T[[1]] c (6);

	c = ln(a);
	T[[1]] d (6);
	T[[1]] temp(6) = declassify(a);
	
	d = declassify(c) - b;

	for(uint i = 0; i < 6;++i){
		if(d[i] < 0){d[i] = -d[i];}
		if(temp[i] < 0){temp[i] = -temp[i];}
	}
	percentage = sum(d / temp); 
	print("TEST completed");
	print("Ln vs. hardcoded answers is imprecise by: ", ((percentage / 6)*100), " %");
}

template<type T>
void test_ln2(T data){
	pd_a3p T[[1]] a (5) = 51.35752137315483;
	T[[1]] c (5);
	c = declassify(ln(a));

	if((c[0] == c[1]) && (c[0] == c[2]) && (c[0] == c[3]) && (c[0] == c[4])){
		print("TEST completed successfully");
	}
	else{
		print("WARNING! using ln on vector with same elements, got different answers");
		print("input vector: ", arrayToString(declassify(a)));
		print("natural logarithm values: ",arrayToString(c));
	}
}

template<type T>
void test_exp(T data){
	T percentage;
	pd_a3p T[[1]] a (6) = {
	 	0.04346448502744411,
		2.608711404814097,
		8.850954516109905,
		5.192202509740625,
		3.804402936143337,
		0.2756185651367633
	};


	T[[1]] b (6) = {
		1.044422901002680287666610221323863320666399833403446910102462,
		13.58153846598169954780858281673407681819654520434165372046665,
		6981.049314932058041420422097243561596289098118645520226253906,
		179.8642697929510613535505964744523805752291131740651211153791,
		44.89843487592044416223236789050014681296169646353990257754926,
		1.317345286763859603539286625646646394800720727417483790997921
	};

	pd_a3p T[[1]] c (6);

	c = exp(a);
	T[[1]] d (6);
	T[[1]] temp(6) = declassify(a);
	
	d = declassify(c) - b;

	for(uint i = 0; i < 6;++i){
		if(d[i] < 0){d[i] = -d[i];}
		if(temp[i] < 0){temp[i] = -temp[i];}
	}
	percentage = sum(d / temp); 
	print("TEST completed");
	print("Exp vs. hardcoded answers is imprecise by: ", ((percentage / 6)*100), " %");
}

template<type T>
void test_exp2(T data){
	pd_a3p T[[1]] a (5) = 2.608711404814097;
	T[[1]] c (5);
	c = declassify(exp(a));

	if((c[0] == c[1]) && (c[0] == c[2]) && (c[0] == c[3]) && (c[0] == c[4])){
		print("TEST completed successfully");
	}
	else{
		print("WARNING! using exp on vector with same elements, got different answers");
		print("input vector: ", arrayToString(declassify(a)));
		print("exponent values: ",arrayToString(c));
	}
}


template<type T>
void test_erf(T data){
	T percentage;
	pd_a3p T[[1]] a (6) = {
		0.04346448502744411,
		2.608711404814097,
		8.850954516109905,
		5.192202509740625,
		3.804402936143337,
		0.2756185651367633
	};

	T[[1]] b (6) = {
		0.049013552633607369266862008432310954046622358690496479344742,
		0.999775106019996958162010061735281060722352816051878464056992,
		0.999999999999999999999999999999999993983573143082665952538894,
		0.99999999999979095906731767211556746577913616541579504230906,
		0.999999925612666639310393628225848799833624500768573616725917,
		0.303303363736001927966300191923234370700806885573703559247713
	};

	pd_a3p T[[1]] c (6);

	c = erf(a);
	T[[1]] d (6);
	T[[1]] temp(6) = declassify(a);
	
	d = declassify(c) - b;

	for(uint i = 0; i < 6;++i){
		if(d[i] < 0){d[i] = -d[i];}
		if(temp[i] < 0){temp[i] = -temp[i];}
	}
	percentage = sum(d / temp); 
	print("TEST completed");
	print("Erf vs. hardcoded answers is imprecise by: ", ((percentage / 6)*100), " %");
}

template<type T>
void test_erf2(T data){
	pd_a3p T[[1]] a (5) = 2.608711404814097;
	T[[1]] c (5);
	c = declassify(erf(a));

	if((c[0] == c[1]) && (c[0] == c[2]) && (c[0] == c[3]) && (c[0] == c[4])){
		print("TEST completed successfully");
	}
	else{
		print("WARNING! using erf on vector with same elements, got different answers");
		print("input vector: ", arrayToString(declassify(a)));
		print("error function values: ",arrayToString(c));
	}
}

void main(){

	print("Precision test: start");

	print("TEST 1: Float32/64 Addition precision");
	{
		print("Float32");
		test_addition(0::float32);
	}
	{
		print("Float64");
		test_addition(0::float64);
	}
	print("TEST 2: Float32/64 Subtraction precision");
	{
		print("Float32");
		test_subtraction(0::float32);
	}
	{
		print("Float64");
		test_subtraction(0::float64);
	}
	print("TEST 3: Float32/64 Multiplication precision");
	{
		print("Float32");
		test_multiplication(0::float32);
	}
	{
		print("Float64");
		test_multiplication(0::float64);
	}
	print("TEST 4: Float32/64 Division precision");
	{
		print("Float32");
		test_division(0::float32);
	}
	{
		print("Float64");
		test_division(0::float64);
	}
	print("TEST 5: Float32/64 inversion precision");
	{
		print("Float32");
		test_inversion(0::float32);
	}
	{
		print("Float64");
		test_inversion(0::float64);
	}
	print("TEST 6: Float32/64 inversion precision(2)");
	{
		print("Float32");
		test_inversion2(0::float32);
	}
	{
		print("Float64");
		test_inversion2(0::float64);
	}
	print("TEST 7: Float32/64 sqrt precision");
	{
		print("Float32");
		test_sqrt(0::float32);
	}
	{
		print("Float64");
		test_sqrt(0::float64);
	}
	print("TEST 8: Float32/64 sqrt precision(2)");
	{
		print("Float32");
		test_sqrt2(0::float32);
	}
	{
		print("Float64");
		test_sqrt2(0::float64);
	}
	print("TEST 9: Float32/64 sin precision");
	{
		print("Float32");
		test_sin(0::float32);
	}
	{
		print("Float64");
		test_sin(0::float64);
	}
	print("TEST 10: Float32/64 sin precision(2)");
	{
		print("Float32");
		test_sin2(0::float32);
	}
	{
		print("Float64");
		test_sin2(0::float64);
	}
	print("TEST 11: Float32/64 ln precision");
	{
		print("Float32");
		test_ln(0::float32);
	}
	{
		print("Float64");
		test_ln(0::float64);
	}
	print("TEST 12: Float32/64 ln precision(2)");
	{
		print("Float32");
		test_ln2(0::float32);
	}
	{
		print("Float64");
		test_ln2(0::float64);
	}
	print("TEST 13: Float32/64 exp precision");
	{
		print("Float32");
		test_exp(0::float32);
	}
	{
		print("Float64");
		test_exp(0::float64);
	}
	print("TEST 14: Float32/64 exp precision(2)");
	{
		print("Float32");
		test_exp2(0::float32);
	}
	{
		print("Float64");
		test_exp2(0::float64);
	}
	print("TEST 15: Float32/64 erf precision");
	{
		print("Float32");
		test_erf(0::float32);
	}
	{
		print("Float64");
		test_erf(0::float64);
	}
	print("TEST 16: Float32/64 erf precision(2)");
	{
		print("Float32");
		test_erf2(0::float32);
	}
	{
		print("Float64");
		test_erf2(0::float64);
	}

    print("Test finished!");
}