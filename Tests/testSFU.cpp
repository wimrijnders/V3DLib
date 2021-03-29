#include "doctest.h"
#include <iostream>
#include <cmath>
#include "V3DLib.h"

using namespace V3DLib;

namespace {

void sfu(Float x, Float::Ptr r) {
  *r = 2*x;                  r += 16;
  *r = V3DLib::exp(3.0f);    r += 16;
  *r = V3DLib::exp(x);       r += 16;
  *r = V3DLib::recip(x);     r += 16;
  *r = V3DLib::recipsqrt(x); r += 16;
  *r = V3DLib::log(x);
}


void check(Float::Array &results, double precision) {
  float val = 1.1f;

  REQUIRE(results[0] == 2*val);
  REQUIRE(abs(results[16*1] - 8.0)           < precision);
  REQUIRE(abs(results[16*2] - exp2(val))     < precision);  // Should be exact, but isn't
  REQUIRE(abs(results[16*3] - 1/val)         < precision);
  REQUIRE(abs(results[16*4] - (1/sqrt(val))) < precision);
  REQUIRE(abs(results[16*5] - log2(val))     < precision);
}

}  // anon namespace


TEST_CASE("Test SFU functions [sfu]") {
  //Platform::use_main_memory(true);  // Remove this when testing on hardware QPUs

  int N = 6;  // Number of results returned

  Float::Array results(16*N);

  auto k = compile(sfu);
  k.load(1.1f, &results);

  INFO("Running interpreter");
  results.fill(0.0);
  k.interpret();
  check(results, 1e-6);

  INFO("Running emulator");
  results.fill(0.0);
  k.emu();
  check(results, 1e-6);

  INFO("Running qpu");
  //
  // For vc4 one of the QPU SFU values are exact! Significant difference with int and emu.
  // Perhaps due to float precision, as opposed to double on cpu?
  //
  // v3d has same output as int and emu.
  //
  double precision = (Platform::has_vc4())?3e-4:1e-6;

  results.fill(0.0);
  k.call();
  check(results, precision);
}
