#include "SourceTranslate.h"
#include <memory>
#include "Support/debug.h"
#include "Support/Platform.h"
#include "vc4/SourceTranslate.h"
#include "v3d/SourceTranslate.h"
#include "Target/instr/Instructions.h"

namespace {

std::unique_ptr<V3DLib::ISourceTranslate> _vc4_source_translate;
std::unique_ptr<V3DLib::ISourceTranslate> _v3d_source_translate;

}  // anon namespace


namespace V3DLib {

/**
 * Used by both v3d and vc4
 *
 * Compare with RECV handling in:
 *   - loadStorePass()
 *   - gather/receive
 */
Instr::List ISourceTranslate::load_var(Var &in_dst, Expr &e) {
  using namespace V3DLib::Target::instr;

  Instr::List ret;

  Reg src = srcReg(e.deref_ptr()->var());
  Reg dst = dstReg(in_dst);

  ret << mov(TMU0_S, src)
      << Instr(TMU0_TO_ACC4)  // TODO is r4 safe? Do we need to select an accumulator in some way?
      << mov(dst, ACC4);

  return ret;
}

/**
 * Generate code to add an offset to the uniforms which are pointers.
 *
 * The calculated offset is assumed to be in ACC0
 */
Instr::List add_uniform_pointer_offset(Instr::List &code) {
  using namespace V3DLib::Target::instr;

  Instr::List ret;

  // offset = 4 * vector_id;
  ret << mov(ACC0, ELEM_ID).comment("Initialize uniform ptr offsets")
      << shl(ACC0, ACC0, 2);            // offset now in ACC0

  // add the offset to all the uniform pointers
  for (int index = 0; index < code.size(); ++index) {
    auto &instr = code[index];

    if (!instr.isUniformLoad()) break;  // Assumption: uniform loads always at top

    if (instr.isUniformPtrLoad()) {
      ret << add(rf((uint8_t) index), rf((uint8_t) index), ACC0);
    }
  }

  ret.back().comment("End initialize uniform ptr offsets");

  return ret;
}


ISourceTranslate &getSourceTranslate() {
  if (Platform::compiling_for_vc4()) {
    if (_vc4_source_translate.get() == nullptr) {
      _vc4_source_translate.reset(new vc4::SourceTranslate());
    }
    return *_vc4_source_translate.get();
  } else {
    if (_v3d_source_translate.get() == nullptr) {
      _v3d_source_translate.reset(new v3d::SourceTranslate());
    }
    return *_v3d_source_translate.get();
  }
}

}  // namespace V3DLib
