/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module x3p_aes_test;

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


void main(){
	print("X3P_AES test: start");

	print("TEST 1: aes128 key generation");
	{
		for(uint i = 50; i < 300; i = i + 50){
			pd_a3p xor_uint32[[1]] key = aes128Genkey(i);

			if(size(key) == (i * 4)){
		    	succeeded_tests = succeeded_tests + 1;
		 		all_tests = all_tests +1;
		 		print("SUCCESS!");
		    }
		    else{
		    	print("FAILURE! aes128 key generation failed, key too big or too small");
		 		all_tests = all_tests +1;
		    }
		}
	}
	print("TEST 2: aes128 key expansion");
	{
		for(uint i = 2; i <= 10; i = i + 2){
			pd_a3p xor_uint32[[1]] key = aes128Genkey(i);
			pd_a3p xor_uint32[[1]] expandedKey = aes128ExpandKey(key);

			if(size(expandedKey) == (i * 44)){
		    	succeeded_tests = succeeded_tests + 1;
		 		all_tests = all_tests +1;
		 		print("SUCCESS!");
		    }
		    else{
		    	print("FAILURE! aes128 expanded key generation failed, key too big or too small");
		 		all_tests = all_tests +1;
		    }
		}
	}
	print("TEST 3: Encrypt/Decrypt with aes128");
	{
		pd_a3p xor_uint32[[1]] plainText = {0x3243f6a8, 0x885a308d, 0x313198a2, 0xe0370734};
		pd_a3p xor_uint32[[1]] expandedKey = {
        0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c, // Round 0
        0xa0fafe17, 0x88542cb1, 0x23a33939, 0x2a6c7605, // Round 1
        0xf2c295f2, 0x7a96b943, 0x5935807a, 0x7359f67f, // Round 2
        0x3d80477d, 0x4716fe3e, 0x1e237e44, 0x6d7a883b, // Round 3
        0xef44a541, 0xa8525b7f, 0xb671253b, 0xdb0bad00, // Round 4
        0xd4d1c6f8, 0x7c839d87, 0xcaf2b8bc, 0x11f915bc, // Round 5
        0x6d88a37a, 0x110b3efd, 0xdbf98641, 0xca0093fd, // Round 6
        0x4e54f70e, 0x5f5fc9f3, 0x84a64fb2, 0x4ea6dc4f, // Round 7
        0xead27321, 0xb58dbad2, 0x312bf560, 0x7f8d292f, // Round 8
        0xac7766f3, 0x19fadc21, 0x28d12941, 0x575c006e, // Round 9
        0xd014f9a8, 0xc9ee2589, 0xe13f0cc8, 0xb6630ca6  // Round 10
    	};

		pd_a3p xor_uint32[[1]] cipherText = aes128EncryptEcb(expandedKey,plainText);
		//pd_a3p xor_uint32[[1]] cipher_control = {0x3925841d, 0x02dc09fb, 0xdc118597, 0x196a0b32};
		pd_a3p xor_uint32[[1]] plainText2 = aes128DecryptEcb(expandedKey,cipherText);
		if(all(declassify(plainText) == declassify(plainText2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
	    }
	    else{
	    	print("FAILURE! encrypting and decrypting with static expanded key gets different end results. Expected: ");
	    	printVector(declassify(plainText));
	    	print(" but got: ");
	    	printVector(declassify(plainText2));
	 		all_tests = all_tests +1;
	    }
	}
	print("TEST 4: aes192 key generation");
	{
		for(uint i = 50; i < 300; i = i + 50){
			pd_a3p xor_uint32[[1]] key = aes192Genkey(i);

			if(size(key) == (i * 6)){
		    	succeeded_tests = succeeded_tests + 1;
		 		all_tests = all_tests +1;
		 		print("SUCCESS!");
		    }
		    else{
		    	print("FAILURE! aes192 key generation failed, key too big or too small");
		 		all_tests = all_tests +1;
		    }
		}
	}
	print("TEST 5: aes192 key expansion");
	{
		for(uint i = 2; i <= 10; i = i + 2){
			pd_a3p xor_uint32[[1]] key = aes192Genkey(i);
			pd_a3p xor_uint32[[1]] expandedKey = aes192ExpandKey(key);

			if(size(expandedKey) == (i * 52)){
		    	succeeded_tests = succeeded_tests + 1;
		 		all_tests = all_tests +1;
		 		print("SUCCESS!");
		    }
		    else{
		    	print("FAILURE! aes192 expanded key generation failed, key too big or too small");
		 		all_tests = all_tests +1;
		    }
		}
	}
	print("TEST 6: Encrypt/Decrypt with aes192");
	{
		pd_a3p xor_uint32[[1]] plainText (4);
		pd_a3p xor_uint32[[1]] expandedKey(52);
		plainText = randomize(plainText);
		expandedKey = randomize(expandedKey);

		pd_a3p xor_uint32[[1]] cipherText = aes192EncryptEcb(expandedKey,plainText);
		pd_a3p xor_uint32[[1]] plainText2 = aes192DecryptEcb(expandedKey,cipherText);
		if(all(declassify(plainText) == declassify(plainText2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
	    }
	    else{
	    	print("FAILURE! encrypting and decrypting with expanded key gets different end results. Expected: ");
	    	printVector(declassify(plainText));
	    	print(" but got: ");
	    	printVector(declassify(plainText2));
	 		all_tests = all_tests +1;
	    }
	}
	print("TEST 7: aes256 key generation");
	{
		for(uint i = 50; i < 300; i = i + 50){
			pd_a3p xor_uint32[[1]] key = aes256Genkey(i);

			if(size(key) == (i * 8)){
		    	succeeded_tests = succeeded_tests + 1;
		 		all_tests = all_tests +1;
		 		print("SUCCESS!");
		    }
		    else{
		    	print("FAILURE! aes256 key generation failed, key too big or too small");
		 		all_tests = all_tests +1;
		    }
		}
	}
	print("TEST 8: aes256 key expansion");
	{
		for(uint i = 2; i <= 10; i = i + 2){
			pd_a3p xor_uint32[[1]] key = aes256Genkey(i);
			pd_a3p xor_uint32[[1]] expandedKey = aes256ExpandKey(key);

			if(size(expandedKey) == (i * 60)){
		    	succeeded_tests = succeeded_tests + 1;
		 		all_tests = all_tests +1;
		 		print("SUCCESS!");
		    }
		    else{
		    	print("FAILURE! aes256 expanded key generation failed, key too big or too small");
		 		all_tests = all_tests +1;
		    }
		}
	}
	print("TEST 9: Encrypt/Decrypt with aes256");
	{
		pd_a3p xor_uint32[[1]] plainText (4);
		pd_a3p xor_uint32[[1]] expandedKey(60);
		plainText = randomize(plainText);
		expandedKey = randomize(expandedKey);

		pd_a3p xor_uint32[[1]] cipherText = aes256EncryptEcb(expandedKey,plainText);
		pd_a3p xor_uint32[[1]] plainText2 = aes256DecryptEcb(expandedKey,cipherText);
		if(all(declassify(plainText) == declassify(plainText2))){
			succeeded_tests = succeeded_tests + 1;
	 		all_tests = all_tests +1;
	 		print("SUCCESS!");
	    }
	    else{
	    	print("FAILURE! encrypting and decrypting with expanded key gets different end results. Expected: ");
	    	printVector(declassify(plainText));
	    	print(" but got: ");
	    	printVector(declassify(plainText2));
	 		all_tests = all_tests +1;
	    }
	}

	print("Test finished!");
	print("Succeeded tests: ", succeeded_tests);
	print("Failed tests: ", all_tests - succeeded_tests);
}