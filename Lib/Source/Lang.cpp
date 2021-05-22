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
  controlStack.push();
  controlStack.push(s);
  stmtStack().push();
  stmtStack().push(mkSkip());  // Prob not necessary any more. TODO scan code for mkSkip() and remove
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

  assertq(controlStack.top()->size() == 1, "Expecting just 1 statement on control stack  for else-block", true);
  Stmt::Ptr s = controlStack.last_stmt();

  if ((s->tag == Stmt::IF || s->tag == Stmt::WHERE ) && s->then_is_null()) {
    s->thenStmt(stmtStack().top()->to_stmt());
    stmtStack().top()->clear();  // reuse top stack item for else-block
    //stmtStack().push(mkSkip());
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

  assert(controlStack.top()->size() == 1);
  Stmt::Ptr s = controlStack.last_stmt();
  auto block = stmtStack().top()->to_stmt();
  stmtStack().pop();

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
    stmtStack().append(s);
    controlStack.pop();
  }
}


//=============================================================================
// 'While' token
//=============================================================================

void While_(Cond c) {
  Stmt::Ptr s = Stmt::mkWhile(c.cexpr(), nullptr);
  controlStack.push();
  controlStack.push(s);
  stmtStack().push();
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
  controlStack.push();
  controlStack.push(s);
  stmtStack().push();
  stmtStack().push(mkSkip());
}


//=============================================================================
// 'For' token
//=============================================================================

void For_(Cond c) {
  Stmt::Ptr s = Stmt::mkFor(c.cexpr(), nullptr, nullptr);
  controlStack.push();
  controlStack.push(s);
  stmtStack().push();
  stmtStack().push(mkSkip());
}


void For_(BoolExpr b) {
  For_(any(b));
}


void ForBody_() {
  assertq(controlStack.top()->size() == 1, "Expecting exactly one statement in for body", true);
  Stmt::Ptr s = controlStack.last_stmt();
  auto inc = stmtStack().top()->to_stmt();

  s->inc(inc);
  stmtStack().top()->clear();  // reuse top stack item for end-block
}


void header(char const *str) {
  //stmtStack().top_stmt()->seq_s1()->header(str);
  stmtStack().last_stmt()->header(str);
}


void comment(char const *str) {
  //stmtStack().top_stmt()->seq_s1()->comment(str);
  stmtStack().last_stmt()->comment(str);
}


void break_point(bool val) {
  if (val) {
    //stmtStack().top_stmt()->seq_s1()->break_point();
    stmtStack().last_stmt()->break_point();
  }
}


void initStmt() {
  controlStack.clear();
}

}  // namespace V3DLib
