#ifndef _QPULIB_V3D_SOURCETRANSLATE_H_
#define _QPULIB_V3D_SOURCETRANSLATE_H_
#include "../SourceTranslate.h"

namespace QPULib {
namespace v3d {

class SourceTranslate : public ISourceTranslate {
public:
	bool deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr *rhs) override;
	void  setupVPMWriteStmt(Seq<Instr>* seq, Stmt *s) override;
};


}  // namespace v3d
}  // namespace QPULib


#endif  // _QPULIB_iV3D_SOURCETRANSLATE_H_
