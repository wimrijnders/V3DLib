#include "catch.hpp"
#include <iostream>
#include "V3DLib.h"

using namespace V3DLib;

namespace {

/**
 * All vector elements calculate the same thing.
 * Just show the first value of a block.
 * /
void show_results(SharedArray<float> &results) {
  for (int i = 0; i < (int) results.size(); i += 16) {
		std::cout << (i >> 4) << ": " << results[i] << std::endl;
	}
}
*/


void sfu(Float x, Ptr<Float> r) {
	*r = 2*x;                  r += 16;
	*r = V3DLib::exp(3.0f);    r += 16;
	*r = V3DLib::exp(x);       r += 16;
	*r = V3DLib::recip(x);     r += 16;
	*r = V3DLib::recipsqrt(x); r += 16;
	*r = V3DLib::log(x);
}


void check(SharedArray<float> &results, double precision) {
	float val = 1.1f;

	REQUIRE(results[0] == 2*val);
	REQUIRE(abs(results[16*1] - 8.0)           < precision);
	REQUIRE(abs(results[16*2] - exp2(val))     < precision);  // Should be exact, but isn't
	REQUIRE(abs(results[16*3] - 1/val)         < precision);
	REQUIRE(abs(results[16*4] - (1/sqrt(val))) < precision);
	REQUIRE(abs(results[16*5] - log2(val))     < precision);
}

}  // anon namespace


TEST_CASE("Test SFU functions", "[sfu]") {
	//Platform::use_main_memory(true);  // Remove this when testing on hardware QPUs

	int N = 6;  // Number of results returned

  SharedArray<float> results(16*N);

	auto k = compile(sfu);
	k.load(1.1f, &results);
	//k.pretty(true);

	INFO("Running interpreter");
	results.fill(0.0);
	k.interpret();
	//show_results(results);
	check(results, 1e-6);

	INFO("Running emulator");
	results.fill(0.0);
	k.emu();
	//show_results(results);
	check(results, 1e-6);

	INFO("Running qpu");
	//
	// For vc4 one of the QPU SFU values are exact! Significant difference with int and emu.
	// Perhaps due to float precision, as opposed to double on cpu?
	//
	// v3d has same output as int and emu.
	//
	double precision = (Platform::instance().has_vc4)?3e-4:1e-6;

	results.fill(0.0);
	//k.pretty(false);
	k.call();
	//show_results(results);
	check(results, precision);
}
