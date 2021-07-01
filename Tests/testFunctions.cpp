#include "doctest.h"
#include <iostream>
#include <V3DLib.h>

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


template<typename T1, typename T2>
void check_result(T1 const &result, T2 const &expected) {
  REQUIRE(expected.size() == result.size());
  for (int i = 0; i < (int) expected.size(); ++i) {
    REQUIRE(expected[i] == result[i]);
  }
}


void sync_kernel(Int::Ptr result, Int::Ptr signal) {
  Int::Ptr output = result - index() + me();
  *output = 0;

  // Let one qpu do extra work, to delay it
  If (me() == 3)
    For (Int i = 0, i < 256, i++)
      *output = i;
    End
  End

  sync_qpus(signal);

  // Let one qpu do extra work, to delay it
  If (me() == 6)
    For (Int i = 0, i < 256, i++)
      *output = i;
    End
  End


  sync_qpus(signal);

  If (me() != 3 && me() != 6)
    *output = me();
  End
}


TEST_CASE("Test qpu sync [funcs][sync]") {
  Platform::use_main_memory(true);

  Int::Array result(16);
  result.fill(-1);

  Int::Array signal(16);
  signal.fill(0);

  auto k = compile(sync_kernel);
  //k.pretty(true, "./obj/test/sync_kernel_vc4.txt");
  k.pretty(false, "./obj/test/sync_kernel_v3d.txt");
  k.setNumQPUs(8);
  k.load(&result, &signal);

  // NOTE: emu() will not work here due to rewriting of pointers
  // TODO can this be fixed?
  k.interpret();  // call();

  //std::cout << "sync result: " << result.dump() << std::endl;
  //std::cout << "sync signal: " << signal.dump() << std::endl;

  std::vector<int> expected = { 0, 1, 2, 255, 4, 5, 255, 7, -1, -1, -1, -1, -1, -1, -1, -1};
  check_result(result, expected);

  Platform::use_main_memory(false);
}
