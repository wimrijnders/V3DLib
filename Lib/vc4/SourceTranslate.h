#ifndef _V3DLIB_VC4_SOURCETRANSLATE_H_
#define _V3DLIB_VC4_SOURCETRANSLATE_H_
#include "../SourceTranslate.h"

namespace V3DLib {
namespace vc4 {

class SourceTranslate : public ISourceTranslate {
  using Parent = ISourceTranslate;

public:
  Instr::List load_var(Var &dst, Expr &e) override;
  Instr::List store_var(Var dst_addr, Var src) override;
  void regAlloc(CFG *cfg, Instr::List &instrs) override;
  bool stmt(Instr::List &seq, Stmt::Ptr s) override; 
};


}  // namespace vc4
}  // namespace V3DLib


#endif  // _V3DLIB_VC4_SOURCETRANSLATE_H_
