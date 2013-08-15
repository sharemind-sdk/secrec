/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

module stat_summary;

import stdlib;
import stat_summary;
import a3p_random;

domain pd_a3p additive3pp;

void test_minimum(){
	pd_a3p int[[1]] data (15);
	pd_a3p uint[[1]] isAvailable (size(data)) = 1;
	int[[1]] control (size(data)); 
	for(uint i = 0; i < 4; ++i){
		data = randomize(data);
		control = declassify(data);
		pd_a3p int result = minimum(data,isAvailable);
		
	}
}


void main(){
	print("Stat_summary test: Start");
	
	print("TEST 1: Minimum");
	{
		test_minimum();
	}
}