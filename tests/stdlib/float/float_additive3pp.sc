/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module float_additive3pp;

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
void test_sum(T data){
    pd_a3p T[[1]] temp (10);
    temp = random(temp); 
    T[[1]] vec = declassify(temp);
    T result = declassify(sum(temp));
    T control = 0;
    for(uint i = 0; i < size(vec);++i){
        control += vec[i];
    }
    if(((control / result) >= 0.99) && ((control / result) <= 1.01)){
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
        print("SUCCESS!");
    }
    else{
        all_tests = all_tests +1;
        print("FAILURE! Sum failed");
    }
}


template<type T>
void test_sum2(T data){
    pd_a3p T[[1]] temp (10); 
    temp = random(temp);
    T[[1]] vec = declassify(temp);
    T[[1]] result = declassify(sum(temp,2::uint));
    uint startingIndex = 0;
    uint endIndex = size(vec) / 2;
    T[[1]] control (2)= 0;
    for(uint i = 0;i < 2;++i){
        for(uint j = startingIndex;j < endIndex;++j){
            control[i] += vec[j];
        }
        startingIndex = 5;
        endIndex = 10;
    }
    if(all((control / result) >= 0.99) && all((control / result) <= 1.01)){
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
        print("SUCCESS!");
    }
    else{
        all_tests = all_tests +1;
        print("FAILURE! Sum failed");
    }
}

void main(){

    print("Standard library: start");

    print("TEST 1: Classify");
    {
        print("float32");
        float32 a = random_float(0::float32); pd_a3p float32 b = classify(a); 
        print("SUCCESS!");
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1; 
    }
    {
        print("float64");
        float64 a = random_float(0::float64); pd_a3p float64 b = classify(a); 
        print("SUCCESS!");
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
    }
    print("TEST 2: Sum");
    {
        print("float32");
        test_sum(0::float32);
    }
    {
        print("float64");
        test_sum(0::float64);
    }
    print("TEST 3: Sum (2)");
    {
        print("float32");
        test_sum2(0::float32);
    }
    {
        print("float64");
        test_sum2(0::float64);
    }

    print("Test finished!");
    print("Succeeded tests: ", succeeded_tests);
    print("Failed tests: ", all_tests - succeeded_tests);
}