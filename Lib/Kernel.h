#ifndef _V3DLIB_KERNEL_H_
#define _V3DLIB_KERNEL_H_
#include <tuple>
#include <algorithm>  // std::move
#include "BaseKernel.h"
#include "Source/Complex.h"
//#include "Support/assign.h"

namespace V3DLib {

enum CompileFor {
  VC4 = 1,
  V3D = 2,
  BOTH = VC4 + V3D
};


// ============================================================================
// Parameter passing
// ============================================================================

template <typename... ts> inline void nothing(ts... args) {}


template <typename T, typename t> inline bool passParam(IntList &uniforms, t x) {
  return T::passParam(uniforms, x);
}


/**
 * Grumbl still need special override for 2D shared array.
 * Sort of patched this, will sort it out another time.
 *
 * You can not possibly have any idea how long it took me to implement and use this correctly.
 * Even so, I'm probably doing it wrong.
 */
template <>
inline bool passParam< Float::Ptr, Float::Array2D * > (IntList &uniforms, Float::Array2D *p) {
  return Float::Ptr::passParam(uniforms, &((BaseSharedArray const &) p->get_parent()));
}


template <>
inline bool passParam< Complex::Ptr, Complex::Array2D * > (IntList &uniforms, Complex::Array2D *p) {
  passParam< Float::Ptr, Float::Array2D * > (uniforms, &p->re());
  passParam< Float::Ptr, Float::Array2D * > (uniforms, &p->im());
  return true;
}


/**
 * API kernel definition.
 *
 * This serves to make the kernel arguments strictly typed.
 *
 * -------------------------
 * NOTES
 * =====
 *
 * 1. A kernel is parameterised by a list of QPU types 'ts' representing
 *    the types of the parameters that the kernel takes.
 *
 *    The kernel constructor takes a function with parameters of QPU
 *    types 'ts'.  It applies the function to constuct an AST.
 *
 * 2. Another way to apply the arguments.
 *
 *    Following allows for custom handling in mkArg.
 *    A consequence is that uniforms are copied to new variables in the source lang generation.
 *    This happens at line 21 in assign.h:
 *
 *            f(std::get<N>(std::forward<Tuple>(t))...);
 *
 *    It's not much of an issue, just ugly.
 *    No need for calling using `apply()` right now, this is for reference.
 * 
 *      // Construct the AST for vc4
 *      auto args = std::make_tuple(mkArg<ts>()...);
 *      apply(f, args);
 */
template <typename... ts> struct Kernel : public BaseKernel {
  using KernelFunction = void (*)(ts... params);

  // Construct an argument of QPU type 't'.
  template <typename T> inline T mkArg() { return T::mkArg(); }

public:
  Kernel(Kernel const &k) = delete;
  Kernel(Kernel &&k) = default;

  /**
   * Construct kernel out of C++ function
   */
  Kernel(KernelFunction f, CompileFor compile_for) {
    if (compile_for & VC4) {
      compile_init(true);
      vc4().compile([this, f] () {
        f(mkArg<ts>()...);  // Construct the AST for vc4; see Note 2 in class header
      });
    }

    if (compile_for & V3D) {
      compile_init(false);

      v3d().compile([this, f] () {
        f(mkArg<ts>()...);  // Construct the AST for v3d
      });
    }
  }


  /**
   * Load uniform values.
   *
   * Pass params, checking arguments types us against parameter types ts.
   */
  template <typename... us>
  Kernel &load(us... args) {
    uniforms.clear();
    nothing(passParam<ts, us>(uniforms, args)...);
    return *this;
  }
};


template <typename... ts>
Kernel<ts...> compile(void (*f)(ts... params), CompileFor compile_for = BOTH) {
  Kernel<ts...> k(f, compile_for);
  return std::move(k);
}

}  // namespace V3DLib

#endif  // _V3DLIB_KERNEL_H_
