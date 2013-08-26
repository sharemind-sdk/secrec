/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module Oblivious_test;

import stdlib;
import matrix;
import additive3pp;
import a3p_matrix;
import oblivious;
import a3p_oblivious;
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
void choice_test1(T data){
    {
        pd_a3p bool cond = true;
        pd_a3p T[[2]] mat (0,0);
        pd_a3p T[[2]] mat2 (0,0);
        pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
        pd_a3p T[[2]] mat4 (0,2);
        pd_a3p T[[2]] mat5 (0,2);
        mat3 = choose(cond,mat4,mat5);
        pd_a3p T[[2]] mat6 (2,0);
        pd_a3p T[[2]] mat7 (2,0);
        mat3 = choose(cond,mat6,mat7);
    }
    pd_a3p bool cond = true;
    pd_a3p T[[2]] mat (3,3);
    pd_a3p T[[2]] mat2 (3,3);
    mat = random(mat); mat2 = random(mat2);
    pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
    if(all(declassify(mat) == declassify(mat3))){
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
        print("SUCCESS!");
    }
    else{
        print("FAILURE! choose test failed. Expected: ", arrayToString(flatten(declassify(mat))), " Got: ",arrayToString(flatten(declassify(mat3))));
        all_tests = all_tests +1;
    }
    cond = false;
    mat = random(mat); mat2 = random(mat2);
    mat3 = choose(cond,mat,mat2);
    if(all(declassify(mat2) == declassify(mat3))){
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
        print("SUCCESS!");
    }
    else{
        print("FAILURE! choose test failed. Expected: ", arrayToString(flatten(declassify(mat2))), " Got: ",arrayToString(flatten(declassify(mat3))));
        all_tests = all_tests +1;
    }
}

template<type T>
void choice_test2(T data){
    {
        pd_a3p bool[[2]] cond (0,0);
        pd_a3p T[[2]] mat (0,0);
        pd_a3p T[[2]] mat2 (0,0);
        pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
        pd_a3p bool[[2]] cond2 (0,2);
        pd_a3p T[[2]] mat4 (0,2);
        pd_a3p T[[2]] mat5 (0,2);
        mat3 = choose(cond2,mat4,mat5);
        pd_a3p bool[[2]] cond3 (2,0);
        pd_a3p T[[2]] mat6 (2,0);
        pd_a3p T[[2]] mat7 (2,0);
        mat3 = choose(cond3,mat6,mat7);
    }
    bool result = true;
    uint column;
    pd_a3p bool[[2]] cond (4,4);
    pd_a3p T[[2]] mat (4,4);
    pd_a3p T[[2]] mat2 (4,4);
    mat = random(mat); mat2 = random(mat2); cond = randomize(cond);
    pd_a3p T[[2]] mat3 = choose(cond,mat,mat2);
    for(uint i = 0; i < 4;++i){
        for(uint j = 0; j < 4; ++j){
            if(declassify(cond[i,j])){
                if(!(declassify(mat3[i,j]) == declassify(mat[i,j]))){
                    result = false;
                    column = j;
                }
            }
            else{
                if(!(declassify(mat3[i,j]) == declassify(mat2[i,j]))){
                    result = false;
                    column = j;
                }
            }
            if(!result){
                break;
            }
        }
        if(!result){
            print("FAILURE! choose test failed. Expected: ", declassify(mat[i,column]), " Got: ", declassify(mat3[i,column]));
            all_tests = all_tests +1;
            break;
        }
    }
    if(result){
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
        print("SUCCESS!");
    }
}

template<type T>
void vector_lookup_test(T data){
    {
        pd_a3p T[[1]] vec (0);
        pd_a3p uint index = 0;
        pd_a3p T scalar = vectorLookup(vec,index);
    }
    bool result = true;
    pd_a3p T scalar;
    pd_a3p T control;
    pd_a3p uint index;
    for(uint i = 3; i < 6;++i){
        pd_a3p T[[1]] vec (i);
        vec = random(vec);
        for(uint j = 0; j < size(vec); ++j){
            index = j;
            scalar = vectorLookup(vec,index);
            if(declassify(scalar) != declassify(vec[j])){
                control = vec[j];
                result = false;
                break;
            }
        }
        if(!result){
            print("FAILURE! vectorLookup failed. Expected: ", declassify(control), " Got: ",declassify(scalar));
            all_tests = all_tests +1;
            result = true;
        }
        else{
            succeeded_tests = succeeded_tests + 1;
            all_tests = all_tests +1;
            print("SUCCESS!");
        }
    }
}

template<type T>
void matrix_row_lookup(T data){
    /**
    \todo matrixLookupRow fails when given an empty vector
    {
        pd_a3p T[[2]] mat (0,0);
        pd_a3p uint index = 0;
        pd_a3p T[[1]] row = matrixLookupRow(mat,index);
        pd_a3p T[[2]] mat2 (2,0);
        row = matrixLookupRow(mat2,index);
        pd_a3p T[[2]] mat3 (0,2);
        row = matrixLookupRow(mat3,index);
    }*/
    bool result = true;
    pd_a3p T[[2]] mat (4,4);
    mat = random(mat);
    T[[2]] mat2 = declassify(mat);
    pd_a3p uint index;
    pd_a3p T[[1]] row;
    public T[[1]] control;
    for(uint i = 0; i < 4; ++i){
        index = i;
        row = matrixLookupRow(mat,index);
        control = mat2[i,:];
        if(!all(declassify(row) == control)){
            result = false;
            break;
        }
    }
    if(!result){
        print("FAILURE! Matrix row lookup failed. Expected: ", arrayToString(control), " Got: ",arrayToString(declassify(row)));
        all_tests = all_tests +1;
    }
    else{
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
        print("SUCCESS!");
    }
}

template<type T>
void matrix_col_lookup(T data){
    /**
    \todo matrixLookupColumn fails when given an empty vector
    {
        pd_a3p T[[2]] mat (0,0);
        pd_a3p uint index = 0;
        pd_a3p T[[1]] row = matrixLookupColumn(mat,index);
        pd_a3p T[[2]] mat2 (2,0);
        row = matrixLookupColumn(mat2,index);
        pd_a3p T[[2]] mat3 (0,2);
        row = matrixLookupColumn(mat3,index);
    }*/
    bool result = true;
    pd_a3p T[[2]] mat (4,4);
    mat = random(mat);
    T[[2]] mat2 = declassify(mat);
    pd_a3p uint index;
    pd_a3p T[[1]] col;
    public T[[1]] control;
    for(uint i = 0; i < 4; ++i){
        index = i;
        col = matrixLookupColumn(mat,index);
        control = mat2[:,i];
        if(!all(declassify(col) == control)){
            result = false;
            break;
        }
    }
    if(!result){
        print("FAILURE! Matrix column lookup failed. Expected: ", arrayToString(control), " Got: ",arrayToString(declassify(col)));
        all_tests = all_tests +1;
    }
    else{
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
        print("SUCCESS!");
    }
}

template<type T>
void matrix_lookup(T data){
    {
        pd_a3p uint index1 = 0;
        pd_a3p uint index2 = 0;
        pd_a3p T[[2]] mat2 (2,0);
        pd_a3p T result = matrixLookup(mat2,index1,index2);
    }
    bool result = true;
    pd_a3p T[[2]] mat (4,4);
    mat = random(mat);
    T[[2]] mat2 = declassify(mat);
    pd_a3p uint row_index;
    pd_a3p uint col_index;
    pd_a3p T nr;
    public T control;
    for(uint i = 0; i < 4; ++i){
        row_index = i;
        for(uint j = 0; j < 4; ++j){
            col_index = j;
            nr = matrixLookup(mat,row_index,col_index);
            control = mat2[i,j];
            if(!(declassify(nr) == control)){
                result = false;
                break;
            }
        }
        if(!result){
            break;
        }   
    }
    if(!result){
        print("FAILURE! Matrix lookup failed. Expected: ", control, " Got: ", declassify(nr));
        all_tests = all_tests +1;
    }
    else{
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
        print("SUCCESS!");
    }
}

template <type T>
void vector_update(T data){
    {
        pd_a3p T[[1]] vec (0);
        pd_a3p uint index = 0;
        pd_a3p T newValue;
        vec = vectorUpdate(vec,index,newValue);
    }
    bool result = true;
    pd_a3p T[[1]] rand (1);
    rand = random(rand);
    pd_a3p T scalar = rand[0];
    pd_a3p uint index;
    pd_a3p T control;
    for(uint i = 3; i < 6; ++i){
        pd_a3p T[[1]] vec (i);
        vec = random(vec);
        for(uint j = 0; j < size(vec);++j){
            index = j;
            vec = vectorUpdate(vec,index,scalar);
            if(declassify(vec[j]) != declassify(scalar)){
                control = vec[j];
                result = false;
                break;
            }
        }
        if(!result){
            print("FAILURE! vector update failed on size ",i," Expected: ", declassify(scalar), " Got: ", declassify(control));
            all_tests = all_tests +1;
            result = true;
        }
        else{
            succeeded_tests = succeeded_tests + 1;
            all_tests = all_tests +1;
            print("SUCCESS!");
        }
    } 
}

template<type T>
void matrix_row_update(T data){
    bool result = true;
    pd_a3p T[[2]] mat (4,4);
    mat = random(mat);

    pd_a3p T[[1]] vec (4);
    vec = random(vec);

    pd_a3p uint row_index;

    for(uint i = 0; i < 4; ++i){
        row_index = i;
        mat = matrixUpdateRow(mat,row_index,vec);
        if(!all(declassify(vec) == declassify(mat[i,:]))){
            result = false;
        }
        if(!result){
            print("FAILURE! Matrix row update failed. Expected: ", arrayToString(declassify(vec)), " Got: ",arrayToString(flatten(declassify(mat[declassify(row_index),:]))));
            all_tests = all_tests +1;
            break;
        }
        else{
            succeeded_tests = succeeded_tests + 1;
            all_tests = all_tests +1;
            print("SUCCESS!");
        }
    }
}

template<type T>
void matrix_col_update(T data){
    bool result = true;
    pd_a3p T[[2]] mat (4,4);
    mat = random(mat);

    pd_a3p T[[1]] vec (4);
    vec = random(vec);

    pd_a3p uint col_index;

    for(uint i = 0; i < 4; ++i){
        col_index = i;
        mat = matrixUpdateColumn(mat,col_index,vec);
        if(!all(declassify(vec) == declassify(mat[:,i]))){
            result = false;
        }
        if(!result){
            print("FAILURE! Matrix column update failed. Expected: ", arrayToString(declassify(vec)), " Got: ",arrayToString(flatten(declassify(mat[:,declassify(col_index)]))));
            all_tests = all_tests +1;
            break;
        }
        else{
            succeeded_tests = succeeded_tests + 1;
            all_tests = all_tests +1;
            print("SUCCESS!");
        }
    }
}

template<type T>
void matrix_update(T data){
    bool result = true;
    pd_a3p T[[2]] mat (4,4);
    mat = random(mat);

    pd_a3p T[[1]] vec (1);
    vec = random(vec);

    pd_a3p T scalar = vec[0];
    pd_a3p uint row_index;
    pd_a3p uint col_index;

    for(uint i = 0; i < 4; ++i){
        row_index = i;
        for(uint j = 0; j < 4; ++j){
            col_index = j;
            mat = matrixUpdate(mat,row_index,col_index,scalar);
            if(!(declassify(scalar) == declassify(mat[i,j]))){
                result = false;
            }
            if(!result){
                break;
            }
        }
        if(!result){
                break;
        }
    }
    if(!result){
        print("FAILURE! Matrix update failed. Expected: ", declassify(scalar), " Got: ",declassify(mat[declassify(row_index),declassify(col_index)]));
        all_tests = all_tests +1;
    }
        else{
        succeeded_tests = succeeded_tests + 1;
        all_tests = all_tests +1;
        print("SUCCESS!");
    }
}

void main(){

    print("Oblivious test: start");
    
    print("TEST 1a: Oblivious choice scalar condition");
    {
        print("float32");
        choice_test1(0::float32);
    }
    {
        print("float64");
        choice_test1(0::float64);
    }
    print("TEST 1b: Oblivious choice matrix condition");
    {
        print("float32");
        choice_test2(0::float32);
    }
    {
        print("float64");
        choice_test2(0::float64);
    }
    print("TEST 2: Oblivious vector lookup with 3-5 element vectors");
    {
        print("float32");
        vector_lookup_test(0::float32);
    }
    {
        print("float64");
        vector_lookup_test(0::float64);
    }
    print("TEST 3: Oblivious matrix row lookup in (4,4) matrix");
    {
        print("float32");
        matrix_row_lookup(0::float32);
    }
    {
        print("float64");
        matrix_row_lookup(0::float64);
    }
    print("TEST 4: Oblivious matrix column lookup in (4,4) matrix");
    {
        print("float32");
        matrix_col_lookup(0::float32);
    }
    {
        print("float64");
        matrix_col_lookup(0::float64);
    }
    print("TEST 5: Oblivious matrix lookup in (4,4) matrix");
    {
        print("float32");
        matrix_lookup(0::float32);
    }
    {
        print("float64");
        matrix_lookup(0::float64);
    }
    print("TEST 6: Oblivious vector update");
    {
        print("float32");
        vector_update(0::float32);
    }
    {
        print("float64");
        vector_update(0::float64);
    }
    print("TEST 7: Oblivious matrix row update");
    {
        print("float32");
        matrix_row_update(0::float32);
    }
    {
        print("float64");
        matrix_row_update(0::float64);
    }
    print("TEST 8: Oblivious matrix column update");
    {
        print("float32");
        matrix_col_update(0::float32);
    }
    {
        print("float64");
        matrix_col_update(0::float64);
    }
    print("TEST 9: Oblivious matrix update");
    {
        print("float32");
        matrix_update(0::float32);
    }
    {
        print("float64");
        matrix_update(0::float64);
    }


    print("Test finished!");
    print("Succeeded tests: ", succeeded_tests);
    print("Failed tests: ", all_tests - succeeded_tests);
}