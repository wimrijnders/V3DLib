#include "Syntax.h"
#include "Common/Heap.h"
#include "Common/Stack.h"
#include "Target/SmallLiteral.h"
#include "Support/basics.h"

namespace V3DLib {

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
//  return astHeap.alloc<Expr>();
breakpoint
	return new Expr;
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
//  return astHeap.alloc<BExpr>();
breakpoint
	return new BExpr;
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
//  return astHeap.alloc<CExpr>();
breakpoint
	return new CExpr;
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
// Class Stmt
// ============================================================================

/**
 * Replacement initializer for this class,
 * because a class with unions can not have a ctor.
 */
void Stmt::init(StmtTag in_tag) {
	clear_comments();  // TODO prob not necessary, check

	assert(SKIP <= in_tag && in_tag <= DMA_START_WRITE);
	tag = in_tag;
}


Stmt::~Stmt() {
	// WRI DEBUG
	breakpoint
}


Stmt *Stmt::create(StmtTag in_tag) {
	breakpoint

	Stmt *ret = new Stmt();
	ret->init(in_tag);

	switch (in_tag) {
		case PRINT: // Version only for string printing
  		ret->print.tag = PRINT_STR;
  		ret->print.str = nullptr;  // NOTE: Needs to be set elsewhere
  		ret->print.expr = nullptr;
		break;
	}

	return ret;
}


Stmt *Stmt::create(StmtTag in_tag, Expr* e0, Expr* e1) {
	breakpoint

	Stmt *ret = new Stmt();
	ret->init(in_tag);

	switch (in_tag) {
		case ASSIGN:
			assertq(e0 != nullptr && e1 != nullptr, "");
			ret->assign.lhs = e0;
			ret->assign.rhs = e1;
		break;
		case STORE_REQUEST:
			assertq(e0 != nullptr && e1 != nullptr, "");
  		ret->storeReq.data = e0;
		  ret->storeReq.addr = e1;
		break;
		case PRINT: // Version only for float and int printing
			assertq(e0 != nullptr && e1 == nullptr, "");
			// NOTE: `print.tag` needs to be set elseqhere
  		ret->print.str = nullptr;  // NOTE: Needs to be set only for printing string
  		ret->print.expr = e0;
		break;
		case DMA_START_READ:
			assertq(e0 != nullptr && e1 == nullptr, "");
  		ret->startDMARead = e0;
		break;
		case DMA_START_WRITE:
			assertq(e0 != nullptr && e1 == nullptr, "");
  		ret->startDMAWrite = e0;
		break;
		default:
			fatal("This tag not handled yet in create(Expr,Expr)");
		break;
	}

	return ret;
}


Stmt *Stmt::create(StmtTag in_tag, Stmt* s0, Stmt* s1) {
	breakpoint

	Stmt *ret = new Stmt();
	ret->init(in_tag);

	switch (in_tag) {
		case SEQ:
			assertq(s0 != nullptr && s1 != nullptr, "");
  		ret->seq.s0 = s0;
		  ret->seq.s1 = s1;
		break;
		case WHERE:
			assertq(s0 != nullptr && s1 != nullptr, "");
			ret->where.cond     = nullptr;  // NOTE: needs to be set elsewhere
			ret->where.thenStmt = s0;
			ret->where.elseStmt = s1;
		break;
		case IF:
			assertq(s0 != nullptr && s1 != nullptr, "");
			ret->ifElse.cond     = nullptr;  // NOTE: needs to be set elsewhere
			ret->ifElse.thenStmt = s0;
			ret->ifElse.elseStmt = s1;
		break;
		case WHILE:
			assertq(s0 != nullptr && s1 == nullptr, "");
			ret->loop.cond = nullptr;  // NOTE: needs to be set elsewhere
			ret->loop.body = s0;
		break;
		case FOR:
			assertq(s0 != nullptr && s1 != nullptr, "");
			ret->forLoop.cond     = nullptr;  // NOTE: needs to be set elsewhere
			ret->forLoop.inc = s0;
			ret->forLoop.body = s1;
		break;
		default:
			fatal("This tag not handled yet in create(Stmt,Stmt)");
		break;
	}

	return ret;
}


// ============================================================================
// Functions on statements
// ============================================================================

// Make a skip statement
Stmt* mkSkip()
{
	return Stmt::create(SKIP);
}

// Make an assignment statement
Stmt* mkAssign(Expr* lhs, Expr* rhs) {
  return Stmt::create(ASSIGN, lhs, rhs);
}

// Make a sequential composition
Stmt* mkSeq(Stmt *s0, Stmt* s1) {
  return Stmt::create(SEQ, s0, s1);
}

Stmt* mkWhere(BExpr* cond, Stmt* thenStmt, Stmt* elseStmt) {
  Stmt* s           = Stmt::create(WHERE, thenStmt, elseStmt);
  s->where.cond     = cond;
  return s;
}

Stmt* mkIf(CExpr* cond, Stmt* thenStmt, Stmt* elseStmt) {
  Stmt* s           = Stmt::create(IF, thenStmt, elseStmt);
  s->ifElse.cond    = cond;
  return s;
}

Stmt* mkWhile(CExpr* cond, Stmt* body) {
  Stmt* s      = Stmt::create(WHILE, body, nullptr);
  s->loop.cond = cond;
  return s;
}

Stmt* mkFor(CExpr* cond, Stmt* inc, Stmt* body) {
  Stmt* s      = Stmt::create(FOR, inc, body);
  s->forLoop.cond = cond;
  return s;
}

Stmt* mkPrint(PrintTag t, Expr* e) {
  Stmt* s      = Stmt::create(PRINT, e, nullptr);
  s->print.tag = t;
  return s;
}

}  // namespace V3DLib
