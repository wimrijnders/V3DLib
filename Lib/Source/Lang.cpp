#include "Lang.h"
#include <stdio.h>
#include "Support/basics.h"  // fatal()
#include "Source/Int.h"
#include "StmtStack.h"

namespace V3DLib {
namespace {

void prepare_stack(Stmt::Ptr s) {
  auto &stack = stmtStack();
  stack.push();
  stack.push(s);
  stack.push();
}

}  // anon namespace

//=============================================================================
// Assignment token
//=============================================================================

void assign(Expr::Ptr lhs, Expr::Ptr rhs) {
  stmtStack() << Stmt::create_assign(lhs, rhs);
}


//=============================================================================
// 'Else' token
//=============================================================================

void Else_() {
  bool ok = false;

  auto block  = stmtStack().top()->to_stmt();
  stmtStack().pop();

  Stmt::Ptr s = stmtStack().last_stmt();

  if ((s->tag == Stmt::IF || s->tag == Stmt::WHERE ) && s->then_is_null()) {
    s->thenStmt(block);
    //stmtStack().top()->clear();  // reuse top stack item for else-block
    stmtStack().push();
    ok = true;
  }

  assertq(ok, "Syntax error: 'Else' without preceeding 'If' or 'Where'");
}


//=============================================================================
// 'End' token
//=============================================================================

void End_() {
  bool ok = false;

  auto block = stmtStack().top()->to_stmt();
  stmtStack().pop();

  Stmt::Ptr s = stmtStack().last_stmt();

  if (s->tag == Stmt::IF || s->tag == Stmt::WHERE) {
    if (s->then_is_null()) {
      s->thenStmt(block);
      ok = true;
    } else if (s->else_is_null()) {
      s->elseStmt(block);
      ok = true;
    }
  }

  if (s->tag == Stmt::WHILE && s->body_is_null()) {
    s->body(block);
    ok = true;
  }

  if (s->tag == Stmt::FOR && s->body_is_null()) {
    s->for_to_while(block);
    ok = true;
  }

  assertq(ok, "Syntax error: unexpected 'End'", true);
  if (ok) {
    stmtStack().pop();
    stmtStack().append(s);
  }
}


//=============================================================================
// Tokens with block handling
//=============================================================================

void If_(Cond c) {
  Stmt::Ptr s = Stmt::create(Stmt::IF);
  s->cond(c.cexpr());
  prepare_stack(s);
}


void While_(Cond c) {
  Stmt::Ptr s = Stmt::create(Stmt::WHILE);
  s->cond(c.cexpr());
  prepare_stack(s);
}


void For_(Cond c) {
  Stmt::Ptr s = Stmt::create(Stmt::FOR);
  s->cond(c.cexpr());
  prepare_stack(s);
}


void If_(BoolExpr b)    { If_(any(b)); }
void While_(BoolExpr b) { While_(any(b)); }
void For_(BoolExpr b)   { For_(any(b)); }


void Where__(BExpr::Ptr b) {
  Stmt::Ptr s = Stmt::create(Stmt::WHERE);
  s->where_cond(b);
  prepare_stack(s);
}


//=============================================================================
// 'For' handling
//=============================================================================

void ForBody_() {
  auto inc = stmtStack().top()->to_stmt();
  stmtStack().pop();

  Stmt::Ptr s = stmtStack().last_stmt();
  s->inc(inc);

  stmtStack().push();
}


void header(char const *str) {
  stmtStack().last_stmt()->header(str);
}


void comment(char const *str) {
  stmtStack().last_stmt()->comment(str);
}


void break_point(bool val) {
  if (val) {
    stmtStack().last_stmt()->break_point();
  }
}

}  // namespace V3DLib
