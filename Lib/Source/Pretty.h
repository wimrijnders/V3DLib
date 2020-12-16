#ifndef _V3DLIB_SOURCE_PRETTY_H_
#define _V3DLIB_SOURCE_PRETTY_H_
#include <cstdio>  // FILE
#include <string>
#include "Source/Stmt.h"

namespace V3DLib {

class Expr;
class CExpr;

// Pretty printer for the V3DLib source language
void pretty(FILE *f, Expr* e);
void pretty(FILE *f, CExpr* c);
void pretty(FILE *f, Stmt::Ptr s);
void pretty(Stmt::Ptr s);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_PRETTY_H_
