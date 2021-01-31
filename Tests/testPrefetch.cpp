#include "catch.hpp"
#include <V3DLib.h>
#include "support/support.h"

namespace {

using namespace V3DLib;

template<typename T>
void prefetch_kernel(Ptr<T> result, Ptr<T> in_src) {
  Ptr<T> src = in_src;
  Ptr<T> dst = result;

  //
  // The usual way of doing things
  //

//  Int input = -2;  comment("Start regular fetch/store");
//  input = *src; //cannot bind non-const lvalue reference of type ‘V3DLib::Int&’ to an rvalue of type ‘V3DLib::Int’
  T input = *src;  comment("Start regular fetch/store");

  src += 16;
  *dst = input;
  dst += 16;

  // See above
//  input = *src;
//  src += 16;
//  *dst = input;
//  dst += 16;
  T inputa = *src;
  src += 16;
  *dst = inputa;
  dst += 16;

  //
  // With regular gather
  //
  //input = (Int) -2; comment("Start regular gather");  // TODO silly that the case is required
  input = -2; comment("Start regular gather");  // TODO silly that the case is required

  gather(src);
  gather(src + 16);
  receive(input);
  store(input, dst);
  dst += 16;
  receive(input);
  store(input, dst);
  dst += 16;


  //
  // Now with prefetch
  //
  *dst = 123;  comment("Start prefetch");
  src += 32;
  input = -3;
  T input2 = -4;
  T a = 1357;             // Interference
  add_prefetch(src);
  T b = 2468;             // Interference
  add_prefetch(src);
  add_prefetch(src + 0);  // For test of usage PointerExpr

  T input3 = -6;
  receive(input);
  receive(input2);
  receive(input3);

  store(input, dst);
  dst += 16;
  store(input2, dst);
  dst += 16;
  store(input3, dst);
}

}  // anon namespace


TEST_CASE("Test prefetch on stmt stack", "[prefetch]") {
  int const N = 7;

  SECTION("Test prefetch with integers") {
    SharedArray<int> src(16*N);
    for (int i = 0; i < (int) src.size(); ++i) {
      src[i] = i + 1;
    }

    SharedArray<int> result(16*N);
    result.fill(-1);

    auto k = compile(prefetch_kernel<Int>);
    //k.pretty(true);
    //k.pretty(false);
    k.load(&result, &src);
    k.interpret();
    //k.emu();  // Failed assertion, DMA not active

    //dump_array(result, 16);
  
    for (int i = 0; i < (int) result.size(); ++i) {
      INFO("i: " << i);
      REQUIRE(result[i] == src[i]);
    }
  }


  SECTION("Test prefetch with floats") {
    SharedArray<float> src(16*N);
    for (int i = 0; i < (int) src.size(); ++i) {
      src[i] = (float) (i + 1);
    }

    SharedArray<float> result(16*N);
    result.fill(-1);

    auto k = compile(prefetch_kernel<Float>);
    k.load(&result, &src);
    k.interpret();

    for (int i = 0; i < (int) result.size(); ++i) {
      INFO("i: " << i);
      REQUIRE(result[i] == src[i]);
    }
  } 
}
