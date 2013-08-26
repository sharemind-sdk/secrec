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

void Success(){
    succeeded_tests = succeeded_tests + 1;
    all_tests = all_tests +1;
    print("SUCCESS!");
}

template<domain D: additive3pp,type T>
void Failure(D T scalar,D T result){
    all_tests = all_tests +1;
    print("IMPRECISE! Expected value ",declassify(scalar)," but got ",declassify(result));
}

template<type T>
void test_inv(T data){
    {
        pd_a3p T[[1]] vec (0);
        pd_a3p T[[1]] result = inv(vec);
        pd_a3p T[[2]] mat (0,0);
        pd_a3p T[[2]] result2 = inv(mat);
        pd_a3p T[[2]] mat2 (2,0);
        pd_a3p T[[2]] result3 = inv(mat2);
        pd_a3p T[[2]] mat3 (2,0);
        pd_a3p T[[2]] result4 = inv(mat3);
    }
    pd_a3p T temp = 71.504533269388;
    pd_a3p T result = inv(temp);
    pd_a3p T scalar = 0.013985127295811784800214762835242144289506641965459514772424;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 0.7164111850802378;
    result = inv(temp);
    scalar = 1.395846436830826912300169456980548765643135686459277164156807;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = -30.40014777649985;
    result = inv(temp);
    scalar = -0.03289457693929460080651200035097043130322734092728779416961;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 29.14530926682408;
    result = inv(temp);
    scalar = 0.034310838524478916201483940525109990585888448227179674751609;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = -3.490183819544366;
    result = inv(temp);
    scalar = -0.28651786029153824876652907594949790036880450793466800071530;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }
}










template<type T>
void test_sqrt(T data){
    {
        pd_a3p T[[1]] vec (0);
        pd_a3p T[[1]] result = sqrt(vec);
        pd_a3p T[[2]] mat (0,0);
        pd_a3p T[[2]] result2 = sqrt(mat);
        pd_a3p T[[2]] mat2 (2,0);
        pd_a3p T[[2]] result3 = sqrt(mat2);
        pd_a3p T[[2]] mat3 (2,0);
        pd_a3p T[[2]] result4 = sqrt(mat3);
    }
    pd_a3p T temp = 2;
    pd_a3p T result = sqrt(temp);
    pd_a3p T scalar = 1.414213562373095048801688724209698078569671875376948073176679;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 0.1;
    result = sqrt(temp);
    scalar = 0.31622776601683793319988935444327185337195551393252168268575048527925;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 0.2520680438028037;
    result = sqrt(temp);
    scalar = 0.5020637845959452;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 1.612666674980346;
    result = sqrt(temp);
    scalar = 1.269908136433634;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 10.74761750718821;
    result = sqrt(temp);
    scalar = 3.278355915270368;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 20.04875104760001;
    result = sqrt(temp);
    scalar = 4.477583170372161;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 7.058176148791843;
    result = sqrt(temp);
    scalar = 2.656722821220129;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }
}









template<type T>
void test_sin(T data){
    {
        pd_a3p T[[1]] vec (0);
        pd_a3p T[[1]] result = sin(vec);
        pd_a3p T[[2]] mat (0,0);
        pd_a3p T[[2]] result2 = sin(mat);
        pd_a3p T[[2]] mat2 (2,0);
        pd_a3p T[[2]] result3 = sin(mat2);
        pd_a3p T[[2]] mat3 (2,0);
        pd_a3p T[[2]] result4 = sin(mat3);
    }
    pd_a3p T temp = 2;
    pd_a3p T result = sin(temp);
    pd_a3p T scalar = 0.909297426825681695396019865911744842702254971447890268378973;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = -2.591158833266627;
    result = sin(temp);
    scalar = -0.52305702219783894117435782687190587225808918400896283367238;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 2.025973655340153;
    result = sin(temp);
    scalar = 0.898183084843072890962281088679208335666350830433122388024890;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 0.05077053980093406;
    result = sin(temp);
    scalar = 0.050748731184247275247749273261571169824032377058418668001890;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }



    temp = 0.1560146834375741;
    result = sin(temp);
    scalar = 0.155382538582007151395195349866599909099990588265240553195932;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }
}










template<type T>
void test_ln(T data){
    {
        pd_a3p T[[1]] vec (0);
        pd_a3p T[[1]] result = ln(vec);
        pd_a3p T[[2]] mat (0,0);
        pd_a3p T[[2]] result2 = ln(mat);
        pd_a3p T[[2]] mat2 (2,0);
        pd_a3p T[[2]] result3 = ln(mat2);
        pd_a3p T[[2]] mat3 (2,0);
        pd_a3p T[[2]] result4 = ln(mat3);
    }
    pd_a3p T temp = 2;
    pd_a3p T result = ln(temp);
    pd_a3p T scalar = 0.693147180559945309417232121458176568075500134360255254120680;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 19.37143870633351;
    result = ln(temp);
    scalar = 2.963799749639153065081482086356157440864483133716907784025809;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 8.01355095537729;
    result = ln(temp);
    scalar = 2.081133978123145341432434262591867363796748211946731326304016;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }
}








template<type T>
void test_exp(T data){
    {
        pd_a3p T[[1]] vec (0);
        pd_a3p T[[1]] result = exp(vec);
        pd_a3p T[[2]] mat (0,0);
        pd_a3p T[[2]] result2 = exp(mat);
        pd_a3p T[[2]] mat2 (2,0);
        pd_a3p T[[2]] result3 = exp(mat2);
        pd_a3p T[[2]] mat3 (2,0);
        pd_a3p T[[2]] result4 = exp(mat3);
    }
    pd_a3p T temp = 4.730296322569438;
    pd_a3p T result = exp(temp);
    pd_a3p T scalar = 113.3291393553677043956554206064549029386122262400736381482935;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 2.217653729055856;
    result = exp(temp);
    scalar = 9.185753296309464529002373228208324050320999152365389053363533;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = -4.943499161180602;
    result = exp(temp);
    scalar = 0.007129607029295815232567517484547170649184452431423495644665;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 0.5891664913460597;
    result = exp(temp);
    scalar = 1.802485401916403216424300563519871037244366347890180754747299;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = -3.374143516929145;
    result = exp(temp);
    scalar = 0.034247438104877426515655546479021676807903158395020214273423;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }
}






template<type T>
void test_erf(T data){
    {
        pd_a3p T[[1]] vec (0);
        pd_a3p T[[1]] result = erf(vec);
        pd_a3p T[[2]] mat (0,0);
        pd_a3p T[[2]] result2 = erf(mat);
        pd_a3p T[[2]] mat2 (2,0);
        pd_a3p T[[2]] result3 = erf(mat2);
        pd_a3p T[[2]] mat3 (2,0);
        pd_a3p T[[2]] result4 = erf(mat3);
    }
    pd_a3p T temp = 4.730296322569438;
    pd_a3p T result = erf(temp);
    pd_a3p T scalar = 0.99999999997762938171481719714979256601818505390828472741213;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 2.217653729055856;
    result = erf(temp);
    scalar = 0.998288685557728913422078693320809998053921317555840630675942;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = -4.943499161180602;
    result = erf(temp);
    scalar = -0.9999999999972738428825420423541876431404271162216781611631;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = 0.5891664913460597;
    result = erf(temp);
    scalar = 0.595272141363452952222309063225099090494893694719914124265248;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }


    temp = -3.374143516929145;
    result = erf(temp);
    scalar = -0.99999817376529374239271282956126531878392026171113368123543;
    if(declassify(isNegligible(scalar - result))){ Success(); }
    else{ Failure(scalar,result); }
}



void main(){

    print("Functions test: start");

    print("TEST 1: Inversion");
    {
        print("float32");
        test_inv(0::float32);
    }
    {
        print("float64");
        test_inv(0::float64);
    }
    print("TEST 2: Square root");
    {
        print("float32");
        test_sqrt(0::float32);
    }
    {
        print("float64");
        test_sqrt(0::float64);
    }
    print("TEST 3: Sin test");
    {
        print("float32");
        test_sin(0::float32);
    }
    {
        print("float64");
        test_sin(0::float64);
    }
    print("TEST 4: Ln test");
    {
        print("float32");
        test_ln(0::float32);
    }
    {
        print("float64");
        test_ln(0::float64);
    }
    print("TEST 5: Exp test");
    {
        print("float32");
        test_exp(0::float32);
    }
    {
        print("float64");
        test_exp(0::float64);
    }
    print("TEST 6: Erf test");
    {
        print("float32");
        test_erf(0::float32);
    }
    {
        print("float64");
        test_erf(0::float64);
    }

    print("Test finished!");
    print("Succeeded tests: ", succeeded_tests);
    print("Failed tests: ", all_tests - succeeded_tests);
}