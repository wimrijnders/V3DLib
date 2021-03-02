/******************************************************************************
 * Immediates are handled fine for vc4, the issue is with v3d,
 * which can only use integers immediates directly in the range [-16..15] inclusive.
 * So some sleight of hand is needed to encode other values.
 *
 * Float constant are also possible, but even more convoluted. (Some) powers of
 * two can be loaded directly, other values have to be constructed.
 * I haven't figured this out yet to full satisfaction.
 ******************************************************************************/
#include "catch.hpp"
#include <V3DLib.h>

using namespace V3DLib;

namespace {

/**
 * Used to check translation of immediate values.
 * This doesn't appear to do anything useful, but it does.
 */
void immediate_kernel(Int::Ptr int_result, Float::Ptr float_result) {
  *int_result   =  25;      int_result.inc();    // failed at one time
  *int_result   =  15;      int_result.inc();    // failed at one time
  *int_result   = -25;                           // Negative values should work as well
  *float_result =  25.0f;   float_result.inc();
  *float_result = -25.0f;   float_result.inc();  // Should work as well for floats that are int's
  *float_result =   0.225f; float_result.inc();  // failed at one time

  // Other test values
  *float_result = 0.0f;
}

}  // anon namespace


TEST_CASE("Test loading of immediates", "[dsl][imm]") {
  int const N = 3;  // Number of distinct results (1 more for float)

  SharedArray<int> int_result(16*N);
  int_result.fill(0);
  SharedArray<float> float_result(16*(N + 1));
  float_result.fill(0.0f);

  auto k = compile(immediate_kernel);
  //k.pretty(false);
  k.load(&int_result, &float_result);
  k.call();

  REQUIRE(int_result[ 0]     ==  25);
  REQUIRE(int_result[16]     ==  15);
  REQUIRE(int_result[16*2]   == -25);
  REQUIRE(float_result[ 0]   ==  25.0f);
  REQUIRE(float_result[16]   == -25.0f);
  REQUIRE(float_result[16*2] ==   0.225f);
  REQUIRE(float_result[16*3] ==   0.0f);
}
