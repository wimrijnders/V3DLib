#include "Source/Stmt.h"
#include <stdio.h>
#include "Support/basics.h"  // fatal()
#include "Common/Stack.h"
#include "Source/Int.h"

namespace QPULib {

namespace {
	Stack<Stmt> *p_stmtStack = nullptr;
	Stack<Stmt> controlStack;
} // anon namespace


Stack<Stmt> &stmtStack() {
	assert(p_stmtStack != nullptr);
	return *p_stmtStack;
}


// Interface to the embedded language.

//=============================================================================
// Assignment token
//=============================================================================

void assign(Expr* lhs, Expr* rhs) {
  Stmt* s = mkAssign(lhs, rhs);
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

//=============================================================================
// 'If' token
//=============================================================================

void If_(Cond c)
{
  Stmt* s = mkIf(c.cexpr, NULL, NULL);
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
  if (controlStack.size > 0) {
    Stmt* s = controlStack.top();
    if (s->tag == IF && s->ifElse.thenStmt == NULL) {
      s->ifElse.thenStmt = stmtStack().top();
      stmtStack().replace(mkSkip());
      ok = 1;
    }
    if (s->tag == WHERE && s->where.thenStmt == NULL) {
      s->where.thenStmt = stmtStack().top();
      stmtStack().replace(mkSkip());
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
  if (controlStack.size > 0) {
    Stmt* s = controlStack.top();
    if (s->tag == IF && s->ifElse.thenStmt == NULL) {
      s->ifElse.thenStmt = stmtStack().top();
      ok = 1;
    }
    else if (s->tag == IF && s->ifElse.elseStmt == NULL) {
      s->ifElse.elseStmt = stmtStack().top();
      ok = 1;
    }
    if (s->tag == WHERE && s->where.thenStmt == NULL) {
      s->where.thenStmt = stmtStack().top();
      ok = 1;
    }
    else if (s->tag == WHERE && s->where.elseStmt == NULL) {
      s->where.elseStmt = stmtStack().top();
      ok = 1;
    }
    if (s->tag == WHILE && s->loop.body == NULL) {
      s->loop.body = stmtStack().top();
      ok = 1;
    }
    if (s->tag == FOR && s->forLoop.body == NULL) {
      // Convert 'for' loop to 'while' loop
      CExpr* whileCond = s->forLoop.cond;
      Stmt* whileBody = mkSeq(stmtStack().top(), s->forLoop.inc);
      s->tag = WHILE;
      s->loop.body = whileBody;
      s->loop.cond = whileCond;
      ok = 1;
    }

    if (ok) {
      stmtStack().pop();
      stmtStack().replace(mkSeq(stmtStack().top(), s));
      controlStack.pop();
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
  Stmt* s = mkWhile(c.cexpr, NULL);
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
  Stmt* s = mkWhere(b, NULL, NULL);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

//=============================================================================
// 'For' token
//=============================================================================

void For_(Cond c)
{
  Stmt* s = mkFor(c.cexpr, NULL, NULL);
  controlStack.push(s);
  stmtStack().push(mkSkip());
}

void For_(BoolExpr b)
{
  For_(any(b));
}

void ForBody_()
{
  Stmt* s = controlStack.top();
  s->forLoop.inc = stmtStack().top();
  stmtStack().pop();
  stmtStack().push(mkSkip());
}

//=============================================================================
// 'Print' token
//=============================================================================

void Print(const char* str)
{
  Stmt* s = mkStmt();
  s->tag = PRINT;
  s->print.tag = PRINT_STR;
  s->print.str = str;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

void Print(IntExpr x)
{
  Stmt* s = mkStmt();
  s->tag = PRINT;
  s->print.tag = PRINT_INT;
  s->print.expr = x.expr;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}


void comment(char const *str) {
 	assert(stmtStack().top() != nullptr);
 	assert(stmtStack().top()->tag == SEQ);
 	assert(stmtStack().top()->seq.s1 != nullptr);
 	stmtStack().top()->seq.s1->comment(str);
}


/**
 * QPU code for clean exit
 */
void finishStmt() {
	assert(p_stmtStack != nullptr);
	p_stmtStack = nullptr;
}


void initStmt(Stack<Stmt> &stmtStack) {
	controlStack.clear();
	stmtStack.clear();
	stmtStack.push(mkSkip());

	assert(p_stmtStack == nullptr);
	p_stmtStack = &stmtStack;
}

}  // namespace QPULib
