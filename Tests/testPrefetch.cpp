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
  input = -2.0f; comment("Start regular gather");

  gather(src);
  gather(src + 16);
  receive(input);
  *dst = input;
  dst += 16;
  receive(input);
  *dst = input;
  dst += 16;


  //
  // Now with prefetch
  //
  *dst = 123;  comment("Start prefetch");
  src += 32;
  input = -3;
  T input2 = -4;
  T a = 1357;             // Interference
  prefetch(input, src);
  T b = 2468;             // Interference
  prefetch(input2, src);
  T input3 = -6;
  prefetch(input3, src + 0);  // For test of usage PointerExpr

  *dst = input;
  dst += 16;
  *dst = input2;
  dst += 16;
  *dst = input3;
}


template<int const N>
void multi_prefetch_kernel(Ptr<Int> result, Ptr<Int> src) {
  Int a = 234;  // Best to have the receiving var out of the loop,
                // otherwise it might be recreated in a different register of the rf
                // (Unproven but probably correct hypothesis)

  for (int i = 0; i < N; ++i) {
    prefetch(a, src);
    *result = 2*a;
    result += 16;
  }
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


  SECTION("Test more fetches than prefetch slots") {
    const int N = 10;  // anything over 8 will result in prefetches after loads

    SharedArray<int> src(16*N);
    for (int i = 0; i < (int) src.size(); ++i) {
      src[i] = i + 1;
    }

    SharedArray<int> result(16*N);
    result.fill(-1);

    auto k = compile(multi_prefetch_kernel<N>);
    //k.pretty(true);
    k.load(&result, &src);
    k.interpret();

    //dump_array(result, 16);

    for (int i = 0; i < (int) result.size(); ++i) {
      INFO("i: " << i);
      REQUIRE(result[i] == 2*src[i]);
    }
  }
}
