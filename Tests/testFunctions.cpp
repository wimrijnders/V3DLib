#include "doctest.h"
#include "Kernel.h"

using namespace V3DLib;

namespace {

void hello_kernel(Int::Ptr result) {
/*
  Def hello()
    Return 42;
  End

  *result = Call hello();
*/

 *result = 42;
}

}  // anon namespace


TEST_CASE("Test functions [funcs]") {
  SUBCASE("Test Hello function") {
    Int::Array result(16);
    result.fill(-1);

    auto k = compile(hello_kernel);
    k.load(&result);
    k.interpret();

    for (int i = 0; i < (int) result.size(); i++) {
      REQUIRE(result[i] == 42);
    }
  }
}
