#include "Lang.h"
#include <stdio.h>
#include "Support/basics.h"  // fatal()
#include "Source/Int.h"
#include "StmtStack.h"

namespace V3DLib {

namespace {
  StmtStack controlStack;
} // anon namespace


// Interface to the embedded language.

//=============================================================================
// Assignment token
//=============================================================================

void assign(Expr::Ptr lhs, Expr::Ptr rhs) {
  stmtStack() << Stmt::create_assign(lhs, rhs);
}


//=============================================================================
// 'If' token
//=============================================================================

void If_(Cond c) {
  Stmt::Ptr s = Stmt::mkIf(c.cexpr(), nullptr, nullptr);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

void If_(BoolExpr b) {
  If_(any(b));
}

//=============================================================================
// 'Else' token
//=============================================================================

void Else_() {
  int ok = 0;

  if (controlStack.size() > 0) {
    Stmt::Ptr s = controlStack.top();

    if ((s->tag == Stmt::IF || s->tag == Stmt::WHERE ) && s->then_is_null()) {
      s->thenStmt(stmtStack().pop());
      stmtStack().push(mkSkip());
      ok = 1;
    }
  }

  if (!ok) {
    fatal("Syntax error: 'Else' without preceeding 'If' or 'Where'");
  }
}

//=============================================================================
// 'End' token
//=============================================================================

void End_() {
  int ok = 0;

  if (!controlStack.empty()) {
    Stmt::Ptr s = controlStack.top();

    if (s->tag == Stmt::IF || s->tag == Stmt::WHERE) {
      if (s->then_is_null()) {
        s->thenStmt(stmtStack().pop());
        ok = 1;
      } else if (s->else_is_null()) {
        s->elseStmt(stmtStack().pop());
        ok = 1;
      }
    }

    if (s->tag == Stmt::WHILE && s->body_is_null()) {
      s->body(stmtStack().pop());
      ok = 1;
    }

    if (s->tag == Stmt::FOR && s->body_is_null()) {
      s->for_to_while(stmtStack().pop());
      ok = 1;
    }

    if (ok) {
      stmtStack().append(controlStack.pop());
    }
  }

  if (!ok) {
    fatal("Syntax error: unexpected 'End'");
  }
}

//=============================================================================
// 'While' token
//=============================================================================

void While_(Cond c) {
  Stmt::Ptr s = Stmt::mkWhile(c.cexpr(), nullptr);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

void While_(BoolExpr b) {
  While_(any(b));
}

//=============================================================================
// 'Where' token
//=============================================================================

void Where__(BExpr::Ptr b) {
  Stmt::Ptr s = mkWhere(b, nullptr, nullptr);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

//=============================================================================
// 'For' token
//=============================================================================

void For_(Cond c) {
  Stmt::Ptr s = Stmt::mkFor(c.cexpr(), nullptr, nullptr);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

void For_(BoolExpr b) {
  For_(any(b));
}

void ForBody_() {
  Stmt::Ptr s = controlStack.top();
  s->inc(stmtStack().pop());
  stmtStack().push(mkSkip());
}

//=============================================================================
// 'Print' token
//=============================================================================

void Print(const char *str) {
  Stmt::Ptr s = Stmt::create(Stmt::PRINT);
  s->print.str(str);
  stmtStack().append(s);
}


void Print(IntExpr x) {
  Stmt::Ptr s = Stmt::create(Stmt::PRINT, x.expr(), nullptr);
  stmtStack().append(s);
}


void header(char const *str) {
   assert(stmtStack().top() != nullptr);
   stmtStack().top()->seq_s1()->header(str);
}


void comment(char const *str) {
   assert(stmtStack().top() != nullptr);
   stmtStack().top()->seq_s1()->comment(str);
}


void initStmt() {
  controlStack.clear();
}

}  // namespace V3DLib
