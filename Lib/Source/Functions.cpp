/******************************************************************************
 * Function Library for functions at the source language level
 *
 * These are not actual functions but generate inlined code.
 * In the kernel code, though, they look like function calls.
 *
 ******************************************************************************/
#include "Functions.h"
#include <iostream>
#include "StmtStack.h"
#include "Lang.h"

namespace V3DLib {
namespace functions {
namespace {

int const MAX_INT = 2147483647;  // Largest positive 32-bit integer that can negated

}  // anon namespace


/**
 * This is the same as negation.
 *
 * Used as an alternative for `-1*a`, because vc4 does 24-bit multiplication only.
 */
IntExpr two_complement(IntExpr a) {
  Stmt::Ptr stmt = tempStmt([a] {
    Int tmp = a;
    tmp = (tmp ^ 0xffffffff) + 1;  // take the 1's complement

    // Prepare an expression which can be assigned
    // dummy is not used, only the rhs matters
    Int dummy;
    dummy = tmp;
  });

  //std::cout << stmt->dump() << std::endl;
  stmtStack() << stmt;

  Stmt *ret = stmt->last_in_seq();
  return ret->assign_rhs();
}


IntExpr abs(IntExpr a) {
  Stmt::Ptr stmt = tempStmt([a] {
    Int tmp = a;

    Where (tmp < 0)
      tmp = (tmp ^ 0xffffffff) + 1;  // take the 1's complement
    End

    // Prepare an expression which can be assigned
    // dummy is not used, only the rhs matters
    Int dummy;
    dummy = tmp;
  });

  //std::cout << stmt->dump() << std::endl;
  stmtStack() << stmt;

  Stmt *ret = stmt->last_in_seq();
  return ret->assign_rhs();
}


/**
 * Adjusted from: https://www.geeksforgeeks.org/divide-two-integers-without-using-multiplication-division-mod-operator/
 *
 * TODO: See if special cases (like division by one can be sanely handled
 */
IntExpr operator/(IntExpr in_a, IntExpr in_b) {

  Stmt::Ptr stmt = tempStmt([in_a, in_b] {
    Int a = in_a;  comment("Start integer divide - Warning: inefficient!");
    Int b = in_b;
    Int sign;

    Where ((a >= 0) != (b >= 0))  // Determine sign
      sign = -1;
    Else
      sign = 1;
    End
 
    a = abs(a);
    b = abs(b);
 
    Int quotient = 0;

    // Divide by continually subtracting and remembering the count
    // Yes, insanely inefficient for large a's and small b's. You tell me how to do better.
    While (any(b != 0 && a >= b))
      Where (b == 0)
        quotient = MAX_INT;
      Else
        Where (a >= b)
          a = a - b;
          quotient += 1;
        End
      End
    End

    Where (sign == -1)
      quotient = two_complement(quotient);
    End
 
    comment("End integer divide");

    // Prepare an expression which can be assigned
    // dummy is not used, only the rhs matters
    Int dummy;
    dummy = quotient;
  });

  //std::cout << stmt->dump() << std::endl;
  stmtStack() << stmt;
  Stmt *ret = stmt->last_in_seq();
  return ret->assign_rhs();
}

}  // namespace functions


/**
 * Sum up all the vector elements of a register.
 *
 * All vector elements of register result will contain the same value.
 */
void rotate_sum(Int &input, Int &result) {
  result = input;              comment("rotate_sum");
  result += rotate(result, 1);
  result += rotate(result, 2);
  result += rotate(result, 4);
  result += rotate(result, 8);
}


void rotate_sum(Float &input, Float &result) {
  result = input;              comment("rotate_sum");
  result += rotate(result, 1);
  result += rotate(result, 2);
  result += rotate(result, 4);
  result += rotate(result, 8);
}


/**
 * Set value of src to vector element 'n' of dst
 *
 * All other values in dst are untouched.
 *
 * @param n  index of vector element to set. Must be in range 0..15 inclusive
 */
void set_at(Int &dst, Int n, Int &src) {
  Where(index() == n)
    dst = src;
  End 
}


void set_at(Float &dst, Int n, Float &src) {
  Where(index() == n)
    dst = src;
  End 
}


}  // namespace V3DLib
