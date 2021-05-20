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
  assert(!controlStack.empty());
  bool ok = false;

  Stmt *s = controlStack.top_stmt();
  assert(s != nullptr);

  if ((s->tag == Stmt::IF || s->tag == Stmt::WHERE ) && s->then_is_null()) {
    s->thenStmt(stmtStack().pop_stmt());
    stmtStack().push(mkSkip());
    ok = true;
  }

  assertq(ok, "Syntax error: 'Else' without preceeding 'If' or 'Where'");
}


//=============================================================================
// 'End' token
//=============================================================================

void End_() {
  assert(!controlStack.empty());
  bool ok = false;

  Stmt *s = controlStack.top_stmt();
  assert(s != nullptr);

  if (s->tag == Stmt::IF || s->tag == Stmt::WHERE) {
    if (s->then_is_null()) {
      s->thenStmt(stmtStack().pop_stmt());
      ok = true;
    } else if (s->else_is_null()) {
      s->elseStmt(stmtStack().pop_stmt());
      ok = true;
    }
  }

  if (s->tag == Stmt::WHILE && s->body_is_null()) {
    s->body(stmtStack().pop_stmt());
    ok = true;
  }

  if (s->tag == Stmt::FOR && s->body_is_null()) {
    s->for_to_while(stmtStack().pop_stmt());
    ok = true;
  }

  assertq(ok, "Syntax error: unexpected 'End'", true);
  if (ok) {
    stmtStack().append(controlStack.pop_stmt());
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
  Stmt *s = controlStack.top_stmt();
  assert(s != nullptr);

  //s->last_in_seq()->inc(stmtStack().pop_stmt());
  s->inc(stmtStack().pop_stmt());
  stmtStack().push(mkSkip());
}


void header(char const *str) {
  assert(stmtStack().top() != nullptr);
  //stmtStack().top_stmt()->seq_s1()->header(str);
  stmtStack().top_stmt()->header(str);
}


void comment(char const *str) {
  assert(stmtStack().top_stmt() != nullptr);
  //stmtStack().top_stmt()->seq_s1()->comment(str);
  stmtStack().top_stmt()->comment(str);
}


void break_point(bool val) {
  if (val) {
    assert(stmtStack().top_stmt() != nullptr);
    //stmtStack().top_stmt()->seq_s1()->break_point();
    stmtStack().top_stmt()->break_point();
  }
}


void initStmt() {
  controlStack.clear();
}

}  // namespace V3DLib
