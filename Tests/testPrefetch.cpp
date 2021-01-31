#include "catch.hpp"
#include <V3DLib.h>
#include "support/support.h"

namespace {

using namespace V3DLib;

void prefetch_kernel(Ptr<Int> result, Ptr<Int> in_src) {
  Ptr<Int> src = in_src;
  Ptr<Int> dst = result;

  //
  // The usual way of doing things
  //

//  Int input = -2;  comment("Start regular fetch/store");
//  input = *src; //cannot bind non-const lvalue reference of type ‘V3DLib::Int&’ to an rvalue of type ‘V3DLib::Int’
  Int input = *src;  comment("Start regular fetch/store");

  src += 16;
  *dst = input;
  dst += 16;

  // See above
//  input = *src;
//  src += 16;
//  *dst = input;
//  dst += 16;
  Int inputa = *src;
  src += 16;
  *dst = inputa;
  dst += 16;

  //
  // With regular gather
  //
  input = (Int) -2; comment("Start regular gather");  // TODO silly that the case is required

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
  Int input2 = -4;
  Int a = index();   // Interference
  add_prefetch(src);
  Int b = 1;         // Interference
  add_prefetch(src + 16);
  src += 2*16;
  receive(input);
  receive(input2);

  store(input, dst);
  dst += 16;
  store(input2, dst);
  dst += 16;
}

}  // anon namespace


TEST_CASE("Test prefetch on stmt stack", "[prefetch]") {
  int const N = 6;

  SharedArray<int> src(16*N);
  for (int i = 0; i < (int) src.size(); ++i) {
    src[i] = i + 1;
  }

  SharedArray<int> result(16*N);
  result.fill(-1);

  auto k = compile(prefetch_kernel);
  //k.pretty(false);
  //k.pretty(true);
  k.load(&result, &src);
  k.interpret();
  //k.emu();  // Failed assertion, DMA not active

  //dump_array(result, 16);
  
  for (int i = 0; i < (int) result.size(); ++i) {
    INFO("i: " << i);
    REQUIRE(result[i] == src[i]);
  }
}
