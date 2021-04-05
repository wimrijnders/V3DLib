#include "doctest.h"
#include "Kernel.h"

using namespace V3DLib;

namespace {

IntExpr hello_function() {
  Stmt::Ptr stmt = tempStmt([] {
    Int Q = 42;

    // Prepare an expression which can be assigned
    // dummy is not used, only the rhs matters
    Int dummy;
    dummy = Q;
  });

  //std::cout << stmt->dump() << std::endl;
  stmtStack() << stmt;
  Stmt *ret = stmt->last_in_seq();
  return ret->assign_rhs();
}


void hello_kernel(Int::Ptr result) {
/*
  Def hello()
    Return 42;
  End

  *result = Call hello();
*/

 //*result = 42;

 *result = hello_function();
}

}  // anon namespace


TEST_CASE("Test functions [funcs]") {
  SUBCASE("Test Hello function") {
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
}
