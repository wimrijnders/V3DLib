#include "Source/Stmt.h"
#include <stdio.h>
#include "Support/basics.h"  // fatal()
#include "Source/Int.h"
#include "Common/StmtStack.h"

namespace V3DLib {

namespace {
	StmtStack *p_stmtStack = nullptr;
	StmtStack controlStack;
} // anon namespace


StmtStack &stmtStack() {
	assert(p_stmtStack != nullptr);
	return *p_stmtStack;
}


// Interface to the embedded language.

//=============================================================================
// Assignment token
//=============================================================================

void assign(Expr* lhs, Expr* rhs) {
  Stmt* s = mkAssign(lhs, rhs);
  stmtStack().append(s);
}

//=============================================================================
// 'If' token
//=============================================================================

void If_(Cond c)
{
  Stmt* s = mkIf(c.cexpr, nullptr, nullptr);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

void If_(BoolExpr b)
{
  If_(any(b));
}

//=============================================================================
// 'Else' token
//=============================================================================

void Else_()
{
  int ok = 0;
  if (controlStack.size() > 0) {
    Stmt* s = controlStack.ttop();

    if (s->tag == IF && s->ifElse.thenStmt == nullptr) {
      s->ifElse.thenStmt = stmtStack().apop();
      stmtStack().push(mkSkip());
      ok = 1;
    }
    if (s->tag == WHERE && s->where.thenStmt == nullptr) {
      s->where.thenStmt = stmtStack().apop();
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

void End_()
{
  int ok = 0;
  if (controlStack.size() > 0) {
    Stmt* s = controlStack.ttop();

    if (s->tag == IF && s->ifElse.thenStmt == nullptr) {
      s->ifElse.thenStmt = stmtStack().apop();
      ok = 1;
    }
    else if (s->tag == IF && s->ifElse.elseStmt == nullptr) {
      s->ifElse.elseStmt = stmtStack().apop();
      ok = 1;
    }
    if (s->tag == WHERE && s->where.thenStmt == nullptr) {
      s->where.thenStmt = stmtStack().apop();
      ok = 1;
    }
    else if (s->tag == WHERE && s->where.elseStmt == nullptr) {
      s->where.elseStmt = stmtStack().apop();
      ok = 1;
    }
    if (s->tag == WHILE && s->loop.body == nullptr) {
      s->loop.body = stmtStack().apop();
      ok = 1;
    }
    if (s->tag == FOR && s->forLoop.body == nullptr) {
      // Convert 'for' loop to 'while' loop
      CExpr* whileCond = s->forLoop.cond;
      Stmt* whileBody = mkSeq(stmtStack().apop(), s->forLoop.inc);
      s->tag = WHILE;
      s->loop.body = whileBody;
      s->loop.cond = whileCond;
      ok = 1;
    }

    if (ok) {
      stmtStack().append(controlStack.apop());
    }
  }

  if (!ok) {
    fatal("Syntax error: unexpected 'End'");
  }
}

//=============================================================================
// 'While' token
//=============================================================================

void While_(Cond c)
{
  Stmt* s = mkWhile(c.cexpr, nullptr);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

void While_(BoolExpr b)
{
  While_(any(b));
}

//=============================================================================
// 'Where' token
//=============================================================================

void Where__(BExpr* b)
{
  Stmt* s = mkWhere(b, nullptr, nullptr);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

//=============================================================================
// 'For' token
//=============================================================================

void For_(Cond c)
{
  Stmt* s = mkFor(c.cexpr, nullptr, nullptr);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

void For_(BoolExpr b) {
  For_(any(b));
}

void ForBody_() {
  Stmt *s = controlStack.ttop();
  s->forLoop.inc = stmtStack().apop();
  stmtStack().push(mkSkip());
}

//=============================================================================
// 'Print' token
//=============================================================================

void Print(const char *str) {
breakpoint
  Stmt* s = Stmt::create(PRINT);
  s->print.str = str;
  stmtStack().append(s);
}

void Print(IntExpr x) {
  Stmt *s = Stmt::create(PRINT, x.expr, nullptr);
  s->print.tag = PRINT_INT;
  stmtStack().append(s);
}


void comment(char const *str) {
 	assert(stmtStack().ttop() != nullptr);
 	assert(stmtStack().ttop()->tag == SEQ);
 	assert(stmtStack().ttop()->seq.s1 != nullptr);
 	stmtStack().ttop()->seq.s1->comment(str);
}


/**
 * QPU code for clean exit
 */
void finishStmt() {
	assert(p_stmtStack != nullptr);
	p_stmtStack = nullptr;
}


void initStmt(StmtStack &stmtStack) {
	controlStack.clear();
	stmtStack.clear();
	stmtStack.push(mkSkip());

	assert(p_stmtStack == nullptr);
	p_stmtStack = &stmtStack;
}

}  // namespace V3DLib
