#include "Source/Syntax.h"
#include "Common/Heap.h"
#include "Common/Stack.h"
#include "Target/SmallLiteral.h"
#include "Params.h"

namespace QPULib {

Heap astHeap("abstract syntax tree", AST_HEAP_SIZE);  // Used for constructing abstract syntax trees


// ============================================================================
// Class Op
// ============================================================================

const char *Op::to_string() {
	switch (op) {
		case ADD:    return "+";
		case SUB:    return "-";
		case MUL:    return "*";
		case MIN:    return " min ";
		case MAX:    return " max ";
		case ROTATE: return " rotate ";
		case SHL:    return " << ";
		case SHR:    return " >> ";
		case USHR:   return " _>> ";
		case ROR:    return " ror ";
		case BOR:    return " | ";
		case BAND:   return " & ";
		case BXOR:   return " ^ ";
		case BNOT:   return "~";
		case ItoF:   return "(Float) ";
		case FtoI:   return "(Int) ";
		// v3d-specific
		case TIDX:   return "tidx";
		case EIDX:   return "eidx";
	}

	assertq(false, "opToString(): unknown opcode", true);
	return nullptr;
}

bool Op::noParams() {
  return (op == TIDX || op == EIDX);
}


bool Op::isUnary() {
  return (op == BNOT || op == ItoF || op == FtoI);
}


// ============================================================================
// Class Expr
// ============================================================================

/**
 * An expression is 'simple' if it is a small literal or a variable.
 */
bool Expr::isSimple() const {
	bool isSmallLit = encodeSmallLit(*this) >= 0;

  return (tag == VAR) || isSmallLit;
}

// ============================================================================
// End Class Expr
// ============================================================================


// Is given operator commutative?
bool isCommutative(Op op) {
  if (op.type != FLOAT) {
    return op.op == ADD
        || op.op == MUL
        || op.op == BOR
        || op.op == BAND
        || op.op == BXOR
        || op.op == MIN
        || op.op == MAX;
  }
  return false;
}

// ============================================================================
// Functions on expressions
// ============================================================================

// Function to allocate an expression
Expr* mkExpr()
{
  return astHeap.alloc<Expr>();
}

// Make an integer literal
Expr* mkIntLit(int lit)
{
  Expr* e   = mkExpr();
  e->tag    = INT_LIT;
  e->intLit = lit;
  return e;
}

// Make a float literal
Expr* mkFloatLit(float lit)
{
  Expr* e     = mkExpr();
  e->tag      = FLOAT_LIT;
  e->floatLit = lit;
  return e;
}

// Make a variable
Expr* mkVar(Var var)
{
  Expr* e = mkExpr();
  e->tag  = VAR;
  e->var  = var;
  return e;
}

// Make an operator application
Expr* mkApply(Expr* lhs, Op op, Expr* rhs)
{
  Expr* e      = mkExpr();
  e->tag       = APPLY;
  e->apply.lhs = lhs;
  e->apply.op  = op;
  e->apply.rhs = rhs;
  return e;
}

// Make a pointer dereference
Expr* mkDeref(Expr* ptr)
{
  Expr* e      = mkExpr();
  e->tag       = DEREF;
  e->deref.ptr = ptr;
  return e;
}

// Is an expression a literal?
bool isLit(Expr* e)
{
  return (e->tag == INT_LIT) || (e->tag == FLOAT_LIT);
}

// ============================================================================
// Functions on boolean expressions
// ============================================================================

// Allocate a boolean expression
BExpr* mkBExpr()
{
  return astHeap.alloc<BExpr>();
}

BExpr* mkNot(BExpr* neg)
{
  BExpr *b    = mkBExpr();
  b->tag      = NOT;
  b->neg      = neg;
  return b;
}

BExpr* mkAnd(BExpr* lhs, BExpr* rhs)
{
  BExpr *b    = mkBExpr();
  b->tag      = AND;
  b->conj.lhs = lhs;
  b->conj.rhs = rhs;
  return b;
}

BExpr* mkOr(BExpr* lhs, BExpr* rhs)
{
  BExpr *b    = mkBExpr();
  b->tag      = OR;
  b->disj.lhs = lhs;
  b->disj.rhs = rhs;
  return b;
}


BExpr* mkCmp(Expr* lhs, CmpOp op, Expr*  rhs) {
  BExpr *b    = mkBExpr();
  b->tag      = CMP;
  b->cmp.lhs  = lhs;
  b->cmp.op   = op;
  b->cmp.rhs  = rhs;
  return b;
}

// ============================================================================
// Functions on conditionals
// ============================================================================

CExpr* mkCExpr()
{
  return astHeap.alloc<CExpr>();
}

CExpr* mkAll(BExpr* bexpr)
{
  CExpr* c = mkCExpr();
  c->tag   = ALL;
  c->bexpr = bexpr;
  return c;
}

CExpr* mkAny(BExpr* bexpr)
{
  CExpr* c = mkCExpr();
  c->tag   = ANY;
  c->bexpr = bexpr;
  return c;
}

// ============================================================================
// Functions on statements
// ============================================================================

// Functions to allocate a statement
Stmt* mkStmt()
{
  return astHeap.alloc<Stmt>();
}

// Make a skip statement
Stmt* mkSkip()
{
  Stmt* s = mkStmt();
  s->tag = SKIP;
  return s;
}

// Make an assignment statement
Stmt* mkAssign(Expr* lhs, Expr* rhs)
{
  Stmt* s       = mkStmt();
  s->tag        = ASSIGN;
  s->assign.lhs = lhs;
  s->assign.rhs = rhs;
  return s;
}

// Make a sequential composition
Stmt* mkSeq(Stmt *s0, Stmt* s1)
{
  Stmt* s   = mkStmt();
  s->tag    = SEQ;
  s->seq.s0 = s0;
  s->seq.s1 = s1;
  return s;
}

Stmt* mkWhere(BExpr* cond, Stmt* thenStmt, Stmt* elseStmt)
{
  Stmt* s           = mkStmt();
  s->tag            = WHERE;
  s->where.cond     = cond;
  s->where.thenStmt = thenStmt;
  s->where.elseStmt = elseStmt;
  return s;
}

Stmt* mkIf(CExpr* cond, Stmt* thenStmt, Stmt* elseStmt)
{
  Stmt* s            = mkStmt();
  s->tag             = IF;
  s->ifElse.cond     = cond;
  s->ifElse.thenStmt = thenStmt;
  s->ifElse.elseStmt = elseStmt;
  return s;
}

Stmt* mkWhile(CExpr* cond, Stmt* body)
{
  Stmt* s      = mkStmt();
  s->tag       = WHILE;
  s->loop.cond = cond;
  s->loop.body = body;
  return s;
}

Stmt* mkFor(CExpr* cond, Stmt* inc, Stmt* body)
{
  Stmt* s         = mkStmt();
  s->tag          = FOR;
  s->forLoop.cond = cond;
  s->forLoop.inc  = inc;
  s->forLoop.body = body;
  return s;
}

Stmt* mkPrint(PrintTag t, Expr* e)
{
  Stmt* s       = mkStmt();
  s->tag        = PRINT;
  s->print.tag  = t;
  s->print.expr = e;
  return s;
}

}  // namespace QPULib
