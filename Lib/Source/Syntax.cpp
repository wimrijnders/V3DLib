#include "Syntax.h"
#include "Common/Stack.h"
#include "Support/basics.h"

namespace V3DLib {


// ============================================================================
// Functions on conditionals
// ============================================================================

CExpr *mkAll(BExpr::Ptr bexpr) {
  return new CExpr(ALL, bexpr);
}


CExpr *mkAny(BExpr::Ptr bexpr) {
  return new CExpr(ANY, bexpr);
}

}  // namespace V3DLib
