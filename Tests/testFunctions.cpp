#include "doctest.h"
#include "Kernel.h"
#include "Source/Functions.h"

using namespace V3DLib;

namespace {

IntExpr hello_function() {
  Stmts stmts = tempStmt([] {
    Int Q = 42;

    // Prepare an expression which can be assigned
    // dummy is not used, only the rhs matters
    Int dummy;
    dummy = Q;
  });

  assert(!stmts.empty());
  assert(stmts.size() == 1);
  auto stmt = stmts[0];
  stmtStack() << stmt;

  Stmt *ret = stmt->last_in_seq();  // TODO prob wrong
  return ret->assign_rhs();
}


// Old-style functions
// TODO: Remove if new is working
void hello_kernel(Int::Ptr result) {
 *result = hello_function();
} 


IntExpr hello2_function() {
  return functions::create_function_snippet([] {
    Int Q = 42;
    functions::Return(Q);
  });
}


void hello2_kernel(Int::Ptr result) {
 *result = hello2_function();
} 
 
}  // anon namespace


TEST_CASE("Test functions [funcs]") {
  SUBCASE("Test hello function") {
    Int::Array result(16);
    result.fill(-1);

    auto k = compile(hello_kernel);
    k.pretty(false, "obj/test/hello_kernel_vc4.txt", false);
    k.load(&result);
    k.interpret();

    for (int i = 0; i < (int) result.size(); i++) {
      REQUIRE(result[i] == 42);
    }
  }

  SUBCASE("Test hello2 function") {
    Int::Array result(16);
    result.fill(-1);

    auto k = compile(hello2_kernel);
    k.load(&result);
    k.interpret();

    for (int i = 0; i < (int) result.size(); i++) {
      REQUIRE(result[i] == 42);
    }
  }
}
