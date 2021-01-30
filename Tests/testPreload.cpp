#include "catch.hpp"
#include <V3DLib.h>

namespace {

using namespace V3DLib;

void preload_kernel(Ptr<Int> src) {
  Int a = index();
  add_preload(src);
  Int b = 1;
  add_preload(src + 16);
}

}  // anon namespace


TEST_CASE("Test preload on stmt stack", "[preload]") {
  SharedArray<int> src(16);
  for (int i = 0; i < (int) src.size(); ++i) {
    src[i] = i + 1;
  }

  auto k = compile(preload_kernel);
  //k.pretty(false);
  k.pretty(true);
  k.load(&src);
}
