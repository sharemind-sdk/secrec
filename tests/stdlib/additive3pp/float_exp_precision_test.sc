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
import test_utility;


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
    percentage = (percentage / 6) * 100;
	print("TEST completed");
	print("Exp vs. hardcoded answers is imprecise by: ", percentage, " %");
    test_report_error(percentage);
}

void main(){

	print("Precision test: start");

	print("TEST 13: Float32/64 exp precision");
	{
		print("Float32");
		test_exp(0::float32);
	}
	{
		print("Float64");
		test_exp(0::float64);
	}

    print("Test finished!");
}
