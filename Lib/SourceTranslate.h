#ifndef _V3DLIB_SOURCETRANSLATE_H_
#define _V3DLIB_SOURCETRANSLATE_H_
#include "Source/Stmt.h"
#include "Target/CFG.h"
#include "Target/instr/Instr.h"

namespace V3DLib {

class ISourceTranslate {
public:
  virtual ~ISourceTranslate() {}

  virtual Instr::List load_var(Var &dst, Expr &e) = 0;
  virtual Instr::List store_var(Var dst_addr, Var src) = 0;
  virtual void regAlloc(CFG *cfg, Instr::List *instrs) = 0;
  virtual bool stmt(Instr::List &seq, Stmt::Ptr s) = 0;
};

ISourceTranslate &getSourceTranslate();
Instr::List add_uniform_pointer_offset(Instr::List &code);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCETRANSLATE_H_
