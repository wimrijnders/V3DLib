#include "catch.hpp"
#include <iostream>
#include "V3DLib.h"

using namespace V3DLib;

namespace {

/**
 * All vector elements calculate the same thing.
 * Just show the first value of a block.
 */
void show_results(SharedArray<float> &results) {
  for (int i = 0; i < results.size(); i += 16) {
		std::cout << (i >> 4) << ": " << results[i] << std::endl;
	}
}


void sfu(Float x, Ptr<Float> results) {
	*results = 2*x; results += 1;
	*results = V3DLib::exp(x);
}

}  // anon namespace


TEST_CASE("Test SFU functions", "[sfu]") {
	Platform::use_main_memory(true);  // Remove this when testing on QPUs

	int N = 2;  // Number of results returned

  SharedArray<float> results(16*N);

	auto k = compile(sfu);
	k.load(1.1f, &results);
	k.pretty(true);

	results.fill(0.0);
	k.interpret();
	REQUIRE(results[16*1] == (float) exp(1.1));
	//show_results(results);

	results.fill(0.0);
	k.emu();
	show_results(results);
/*
	results.fill(0.0);
	k.call();
	show_results(results);
*/
}
