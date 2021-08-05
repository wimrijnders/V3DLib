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
#include "LibSettings.h"

namespace V3DLib {
namespace functions {
namespace {

int const MAX_INT = 2147483647;  // Largest positive 32-bit integer that can be negated

} // anon namespace


/**
 * Ensure a common exit method for function snippets.
 *
 * The return value should actually be an IntExpr instance, which is derived downstream
 * from the `dummy` variable as defined here.
 *
 * This is more of a semantics thing; it's a tiny bit of code, but the name implies
 * what the intention is.
 */
void Return(Int const &val) {
  // Prepare an expression which can be assigned
  // dummy is not used downstream, only the rhs matters
  Int dummy;
  dummy = val;
}


void Return(Float const &val) {
  // Prepare an expression which can be assigned
  // dummy is not used downstream, only the rhs matters
  Float dummy;
  dummy = val;
}


/**
 * Create a function snippet from the generation of the passed callback
 *
 * This hijacks the global statement stack to generate from source lang,
 * and then isolates the generation in a separate expression.
 *
 * The immediate benefit of this is to be able to define source lang
 * constructs using the source lang itself.

 * This can be done to some extent directly, but defining them as standalone code
 * is more flexible.
 * The code snippets are relocatable and can be inserted anywhere
 *
 * Potential other uses:
 *   - memoization
 *   - true functions (currently everything generated inline)
 *
 * Because this uses the global statement stack, it is **not** threadsafe.
 * But then again, nothing using the global statement stack is.
 */
IntExpr create_function_snippet(StackCallback f) {
  auto stmts = tempStmt(f);
  assert(!stmts.empty());
  stmtStack() << stmts;
  Stmt::Ptr ret = stmts.back();
  return ret->assign_rhs();
}


// TODO see if this can be merged with the Int version.
FloatExpr create_float_function_snippet(StackCallback f) {
  auto stmts = tempStmt(f);

  assert(!stmts.empty());

  // Return only the assign part of the final statement and remove that statement
  auto stmt = stmts.back();
  stmts.pop_back();
  stmtStack() << stmts;
  return stmt->assign_rhs();
}


/**
 * This is the same as negation.
 *
 * Used as an alternative for `-1*a`, because vc4 does 24-bit multiplication only.
 */
IntExpr two_complement(IntExpr a) {
  return create_function_snippet([a] {
    Int tmp = a;
    tmp = (tmp ^ 0xffffffff) + 1;  // take the 1's complement

    Return(tmp);
  });
}


IntExpr abs(IntExpr a) {
  return create_function_snippet([a] {
    Int tmp = a;

    Where (tmp < 0)
      tmp = (tmp ^ 0xffffffff) + 1;  // take the 1's complement
    End

    Return(tmp);
  });
}


/**
 * Determine index of topmost bit set.
 *
 * Incoming values are assumed to be unsigned.
 * a == 0 will return -1.
 */
IntExpr topmost_bit(IntExpr in_a) {
  return create_function_snippet([in_a] {
    Int a = in_a;
    Int topmost = -1;

    For (Int n = 30, n >= 0, n--)
      Where (topmost == -1)
        Where ((a & (1 << n)) != 0)
          topmost = n;
        End
      End
    End

    Return(topmost);
  });
}


/**
 * Long integer division, returning quotient and remainder
 *
 * There is no support for hardware integer division on the VideoCores, this is an implementation for
 * when you really need it (costly).
 *
 * Source: https://en.wikipedia.org/wiki/Division_algorithm#Integer_division_(unsigned)_with_remainder
 */
void integer_division(Int &Q, Int &R, IntExpr in_a, IntExpr in_b) {
  Int N = in_a;  comment("Start long integer division");
  Int D = in_b;

  Int sign = 1;

  Where ((N >= 0) != (D >= 0))       // Determine sign
    sign = -1;
  End
 
  N = abs(N);
  D = abs(D);

  Q = 0;                             // Initialize quotient and remainder to zero
  R = 0;

  IntExpr top_bit = topmost_bit(N);  // Find first non-zero bit


  For (Int i = 30, i >= 0, i--)
    Where (D == 0)
      Q = MAX_INT;                   // Indicates infinity
    Else
      Where (top_bit >= i)
        R = R << 1;                  // Left-shift R by 1 bit (lsb == 0)
        R |= (N >> i) & 1;           // Set the least-significant bit of R equal to bit i of the numerator
        Where (R >= D)
          R -= D;
          Q |= (1 << i);
        End
      End
    End
  End

  Where (sign == -1)
    Q = two_complement(Q);
  End
 
  comment("End long integer division");  // For some reason, phantom instances of this comment can pop up elsewhere
                                         // in code dumps. Not bothering with correcting this.
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
  extra_precision |= LibSettings::use_high_precision_sincos(); // setting to true in param overrides lib setting

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
  extra_precision |= LibSettings::use_high_precision_sincos(); // setting to true in param overrides lib setting

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
 * Calculate sine for v3d using  hardware
 * 
 * Use this for v3d only.
 *
 * v3d sin takes params in which are multiples of PI.
 * Also works only in range -PI/2..PI/2.
 *
 * Incoming values are multiples of 2*PI.
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * In `DotVector::dft_dot_product()`:
 *
 *    Complex tmp1(elements[i]*cos(param), elements[i]*sin(param));
 *
 *   ... without `create_function_snippet()`, the calculation was split as follows:
 *
 *     - calc var tmp for cos()
 *     - calc var tmp for sin()
 *     - call sin() for cos
 *     - mult cos result with elements[i]
 *     - call sin() for sin
 *     - mult sin result with elements[i]
 *
 *   I.e., the sin() calls are delayed till they are actually used.
 *
 *   IMO this is because the tmp calculation is added immediately to the statement stack.
 *   However, the addition of the sin() operation is delayed; it is returned as an expression and 
 *   added directly after the current function has returned.
 *
 *   It's a bit of a stretch of imagination to see this happening, but it's definitely possible.
 *
 *   Nice to see that the calculation works fine, even with this happening.
 *  
 *   ... with `create_function_snippet()`, the calculation has the expected order:
 *
 *     - calc var tmp for cos()
 *     - call sin() for cos
 *     - calc var tmp for sin()
 *     - call sin() for sin
 *     - mult cos result with elements[i]
 *     - mult sin result with elements[i]
 * 
 *   This is yet another reason for using function snippets.
 *
 *   Okay, that was real interesting.
 */
FloatExpr sin_v3d(FloatExpr x_in) {
  //debug("using v3d sin");

  return create_float_function_snippet([x_in] {
    Float tmp = x_in;                    comment("Start source lang v3d sin");

    tmp += 0.25f;                        // Modulo to range -0.25...0.75
    comment("v3d sin preamble to get param in the allowed range");

    tmp -= functions::ffloor(tmp);       // Get the fractional part
    tmp -= 0.25f;

    Where (tmp > 0.25f)                  // Adjust value to the range -PI/2...PI/2
      tmp = 0.5f - tmp;
    End

    tmp *= 2;                            // Convert to multiple of PI
    comment("End v3d sin preamble");

    Return(sin_op(tmp));
  });
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
    comment("Start ffloor()");

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


/**
 * Let QPUs wait for each other.
 *
 * Intended for v3d, where I don't see a hardware signal function as in vc4.
 * Works fine with vc4 also.
 */
void sync_qpus(Int::Ptr signal) {
  If (numQPUs() != 1) // Don't bother syncing if only one qpu
    *(signal - index() + me()) = 1;

    header("Start QPU sync");

    If (me() == 0)
      Int expected = 0;   comment("QPU 0: Wait till all signals are set");
      Where (index() < numQPUs())
        expected = 1;
      End 

      Int tmp = *signal;
      While (expected != tmp)
        tmp = *signal;
      End

      *signal = 0;        comment("QPU 0 done waiting, let other qpus continue");
    Else
      Int tmp = *signal;  comment("Other QPUs: Wait till all signals are cleared");

      While (0 != tmp)
        tmp = *signal;
      End
    End
  End
}

}  // namespace V3DLib
