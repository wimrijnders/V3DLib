#include "doctest.h"
#include "Kernel.h"
#include "Source/Functions.h"

using namespace V3DLib;

namespace {

IntExpr hello_int_function() {
  return functions::create_function_snippet([] {
    Int Q = 42;
    functions::Return(Q);
  });
}


void hello_int_kernel(Int::Ptr result) {
 *result = hello_int_function();
} 


FloatExpr hello_float_function() {
  return functions::create_float_function_snippet([] {
    Float Q = 42;
    functions::Return(Q);
  });
}


void hello_float_kernel(Float::Ptr result) {
 *result = hello_float_function();
} 
 
}  // anon namespace


TEST_CASE("Test functions [funcs]") {
  SUBCASE("Test hello int function") {
    Int::Array result(16);
    result.fill(-1);

    auto k = compile(hello_int_kernel);
    k.load(&result);
    k.interpret();

    for (int i = 0; i < (int) result.size(); i++) {
      REQUIRE(result[i] == 42);
    }
  }

  SUBCASE("Test hello float function") {
    Float::Array result(16);
    result.fill(-1);

    auto k = compile(hello_float_kernel);
    k.load(&result);
    k.interpret();

    for (int i = 0; i < (int) result.size(); i++) {
      REQUIRE(result[i] == 42.0f);
    }
  }
}
