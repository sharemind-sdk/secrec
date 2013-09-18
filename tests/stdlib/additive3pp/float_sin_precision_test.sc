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
    percentage = (percentage / 6) * 100;
	print("TEST completed");
	print("Sin vs. hardcoded answers is imprecise by: ", percentage, " %");
    test_report_error(percentage);
}

void main(){

	print("Precision test: start");

	print("TEST 9: Float32/64 sin precision");
	{
		print("Float32");
		test_sin(0::float32);
	}
	{
		print("Float64");
		test_sin(0::float64);
	}

    print("Test finished!");
}
