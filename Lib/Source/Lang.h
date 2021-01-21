#ifndef _V3DLIB_SOURCE_LANG_H_
#define _V3DLIB_SOURCE_LANG_H_
#include "Source/Cond.h"
#include "Source/Ptr.h"

namespace V3DLib {

class StmtStack;  // Forward declaration

//=============================================================================
// Statement macros
//=============================================================================

#define If(c)    If_(c); {
#define Else     } Else_(); {
#define End      } End_();
#define While(c) While_(c); {
#define Where(b) Where_(b); {
#define For(init, cond, inc) \
  { init;                    \
    For_(cond);              \
      inc;                   \
    ForBody_();

//=============================================================================
// Statement tokens
//=============================================================================

void assign(Expr::Ptr lhs, Expr::Ptr rhs);
void If_(Cond c);
void If_(BoolExpr c);
void Else_();
void End_();
void While_(Cond c);
void While_(BoolExpr b);
void Where__(BExpr::Ptr b);
inline void Where_(BoolExpr b) { Where__(b.bexpr()); }
void For_(Cond c);
void For_(BoolExpr b);
void ForBody_();
void Print(const char *);
void Print(IntExpr x);
void header(char const *str);
void comment(char const *str);
void initStmt(StmtStack &stmtStack);
void finishStmt();

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_LANG_H_
