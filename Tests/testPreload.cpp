#include "catch.hpp"
#include <V3DLib.h>
#include "support/support.h"

namespace {

using namespace V3DLib;

void preload_kernel(Ptr<Int> result, Ptr<Int> in_src) {
  Ptr<Int> src = in_src;
  Ptr<Int> dst = result;

  //
  // The usual way of doing things
  //
  Int input = -2;  comment("Start regular fetch/store");
  input = *src;
  src += 16;
  *dst = input;
  dst += 16;

  input = *src;
  src += 16;
  *dst = input;
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
  // Now with preload
  //
  *dst = 123;  comment("Start with preload");
  src += 32;
  input = (Int) -3;  // TODO silly that the case is required
  Int input2 = -4;
  Int a = index();   // Interference
  add_preload(src);
  Int b = 1;         // Interference
  add_preload(src + 16);
  src += 2*16;
  receive(input);
  receive(input2);

  store(input, dst);
  dst += 16;
  store(input2, dst);
  dst += 16;
}

}  // anon namespace


TEST_CASE("Test preload on stmt stack", "[preload]") {
  int const N = 6;

  SharedArray<int> src(16*N);
  for (int i = 0; i < (int) src.size(); ++i) {
    src[i] = i + 1;
  }

  SharedArray<int> result(16*N);
  result.fill(-1);

  auto k = compile(preload_kernel);
  //k.pretty(false);
  k.pretty(true);
  k.load(&result, &src);
  k.interpret();  // preload  only returning first values of loaded vector in all vector slots
  // k.emu();  Failed assertion

  dump_array(result, 16);
  
  for (int i = 0; i < (int) result.size(); ++i) {
    INFO("i: " << i);
    REQUIRE(result[i] == src[i]);
  }
}
