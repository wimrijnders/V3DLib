/******************************************************************************
 * Function Library for functions at the source language level
 *
 * These are not actual functions but generate inlined code.
 * In the kernel code, though, they look like function calls.
 *
 ******************************************************************************/
#include "Functions.h"
#include <iostream>
#include <cmath>
#include "Support/Platform.h"
#include "StmtStack.h"
#include "Lang.h"

namespace V3DLib {
namespace functions {
namespace {

int const MAX_INT = 2147483647;  // Largest positive 32-bit integer that can be negated

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


///////////////////////////////////////////////////////////////////////////////
// Trigonometric functions
///////////////////////////////////////////////////////////////////////////////

/**
 * scalar version of cosine
 *
 * The input param is normalized on 2*M_PI. Hence setting `x = 1.0f` means that
 * `cos(2*M_PI) is calculated.
 *
 * Source: https://stackoverflow.com/questions/18662261/fastest-implementation-of-sine-cosine-and-square-root-in-c-doesnt-need-to-b/28050328#28050328
 */
float cos(float x_in, bool extra_precision) noexcept {
  double x = x_in;

  x -= .25 + std::floor(x + .25);
  x *= 16. * (std::abs(x) - .5);

  if (extra_precision) {
    x += .225 * x * (std::abs(x) - 1.);
  }

  return (float) x;
}


/**
 * scalar version of sine
 *
 * NB: The input param is normalized on 2*M_PI.
 */
float sin(float x_in, bool extra_precision) noexcept {
  return functions::cos(0.25f - x_in, extra_precision);
}


FloatExpr cos(FloatExpr x_in, bool extra_precision) {
  Float x = x_in;

  x -= 0.25f + functions::ffloor(x + 0.25f);  comment("Start cosine");
  x *= 16.0f * (fabs(x) - 0.5f);

  if (extra_precision) {
    x += 0.225f * x * (fabs(x) - 1.0f);
  }

  return x;
}


FloatExpr sin(FloatExpr x_in, bool extra_precision) {
  return cos(0.25f - x_in, extra_precision);
}


/**
 * Implementation of ffloor() in source language.
 *
 * This is meant specifically for vc4; v3d actually has an ffloor operation.
 *
 * Relies on IEEE 754 specs for 32-bit floats.
 * Special values (Nan's, Inf's) are ignored
 */
FloatExpr ffloor(FloatExpr x) {
  Float ret;

  if (Platform::compiling_for_vc4()) {
    int const SIZE_MANTISSA = 23;

    Int exp           = ((x.as_int() >> SIZE_MANTISSA) & ((1 << 8) - 1)) - 127;  comment("Calc exponent"); 
    Int fraction_mask = (1 << (SIZE_MANTISSA - exp)) - 1;
    Int frac          = x.as_int() & fraction_mask;                              comment("Calc fraction"); 

    //
    // Clear the fractional part of the mantissa
    //
    // Helper for better readability
    //
    auto zap_mantissa  = [&fraction_mask] (FloatExpr x) -> FloatExpr {
      Float ret;
      ret.as_float(x.as_int() & ~(fraction_mask + 0));
      return ret;
    };

    ret = x;  // result same as input for exp > 23 bits and whole-integer negative values

    Where (exp <= 23)                               // Doesn't work, expecting SEQ: comment("Start ffloor()");
      Where (x >= 1)
        ret = zap_mantissa(x);
      Else Where (x >= 0)
        ret = 0.0f;
      Else Where (x >= -1.0f)
        ret = -1.0f;
      Else Where (x < -1.0f && (frac != 0))
        ret = zap_mantissa(x) - 1;
      End End End End
    End
  } else {
    // v3d
    ret = V3DLib::ffloor(x);  comment("ffloor() v3d");
  }

  return ret;
}


/**
 * Implementation of fabs() in source language.
 *
 * Relies on IEEE 754 specs for 32-bit floats.
 * Special values (Nan's, Inf's) are ignored
 */
FloatExpr fabs(FloatExpr x) {
  Float ret;

  if(Platform::compiling_for_vc4()) {
    uint32_t const Mask = ~(((uint32_t) 1) << 31);

    // Just zap the top bit
    ret.as_float(x.as_int() & Mask);            comment("fabs vc4");
  } else {
    // v3d: The conversion of Mask is really long-winded; make the mask in-place
    ret.as_float(x.as_int() & shr(Int(-1), 1));  comment("fabs v3d");
  }

  return ret;
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
void set_at(Int &dst, Int n, Int const &src) {
  Where(index() == n)
    dst = src;
  End 
}


void set_at(Float &dst, Int n, Float const &src) {
  Where(index() == n)
    dst = src;
  End 
}


}  // namespace V3DLib
