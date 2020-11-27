#include "Syntax.h"
#include "Common/Stack.h"
#include "Support/basics.h"

namespace V3DLib {
/*
BExpr::	BExpr(BExpr const &rhs) {
	switch(rhs.tag() {
	case NOT:
		break;
	case AND:
		break;
	case OR:
		break;
	case CMP:
		break;
	default:
		assert(false);
		break;
	}
}
*/


BExpr::BExpr(Expr::Ptr lhs, CmpOp op, Expr::Ptr rhs) {
 	m_tag      = CMP;
  m_cmp_lhs  = lhs;
  cmp.op   = op;
  m_cmp_rhs  = rhs;
}


Expr::Ptr BExpr::cmp_lhs() { assert(m_tag == CMP && m_cmp_lhs.get() != nullptr); return  m_cmp_lhs; }
Expr::Ptr BExpr::cmp_rhs() { assert(m_tag == CMP && m_cmp_rhs.get() != nullptr); return  m_cmp_rhs; }

void BExpr::cmp_lhs(Expr::Ptr p) { assert(m_tag == CMP); m_cmp_lhs = p; }
void BExpr::cmp_rhs(Expr::Ptr p) { assert(m_tag == CMP); m_cmp_rhs = p; }


/**
 * Create a new instance which encapsulates the current instance with
 * a negation.
 *
 * `not` is a keyword, hence capital.
 */
BExpr *BExpr::Not() const {
  BExpr *b = new BExpr();
  b->m_tag = NOT;
  b->neg   = const_cast<BExpr *>(this);
  return b;
}


/**
 * Create a new instance which combines the current instance with
 * the passed instance to an and-operation.
 *
 * `and` is a keyword, hence capital.
 */
BExpr *BExpr::And(BExpr *rhs) const {
  BExpr *b = new BExpr();
  b->m_tag = AND;
  b->conj.lhs = const_cast<BExpr *>(this);
  b->conj.rhs = rhs;
  return b;
}


/**
 * Create a new instance which combines the current instance with
 * the passed instance to an or-operation.
 *
 * `or` is a keyword, hence capital.
 */
BExpr *BExpr::Or(BExpr *rhs) const {
  BExpr *b    = new BExpr();
  b->m_tag     = OR;
  b->disj.lhs = const_cast<BExpr *>(this);
  b->disj.rhs = rhs;
  return b;
}


// ============================================================================
// Functions on conditionals
// ============================================================================

CExpr *mkCExpr() {
	breakpoint
	return new CExpr;
}

CExpr* mkAll(BExpr* bexpr) {
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
	assertq(tag == SKIP, "Stmt::init(): can't reassign tag once assigned");
	tag = in_tag;
}


Stmt::~Stmt() {
	// WRI DEBUG
	breakpoint
}


Stmt *Stmt::create(StmtTag in_tag) {
	Stmt *ret = new Stmt();
	ret->init(in_tag);

	switch (in_tag) {
		case PRINT: // Version only for string printing
  		ret->print.tag = PRINT_STR;
  		ret->print.str = nullptr;  // NOTE: Needs to be set elsewhere
  		ret->print.expr = nullptr;
		break;
	}

	// WRI debug
	// break for the tags we haven't checked yet
	switch (in_tag) {
		case SKIP:
			break;
		default:
			breakpoint;
			break;
	}

	return ret;
}


Stmt *Stmt::create(StmtTag in_tag, Expr::Ptr e0, Expr::Ptr e1) {
	// WRI debug
	// break for the tags we haven't checked yet
	switch (in_tag) {
		case ASSIGN:
			break;
		default:
			breakpoint;
			break;
	}

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

Stmt *mkSkip() { return Stmt::create(SKIP); }
Stmt *mkAssign(Expr::Ptr lhs, Expr::Ptr rhs) { return Stmt::create(ASSIGN, lhs, rhs); }
Stmt *mkSeq(Stmt *s0, Stmt* s1) { return Stmt::create(SEQ, s0, s1); }

Stmt *mkWhere(BExpr *cond, Stmt *thenStmt, Stmt *elseStmt) {
  Stmt* s           = Stmt::create(WHERE, thenStmt, elseStmt);
  s->where.cond     = cond;
  return s;
}


Stmt *mkIf(CExpr *cond, Stmt *thenStmt, Stmt *elseStmt) {
  Stmt* s           = Stmt::create(IF, thenStmt, elseStmt);
  s->ifElse.cond    = cond;
  return s;
}


Stmt *mkWhile(CExpr *cond, Stmt *body) {
  Stmt* s      = Stmt::create(WHILE, body, nullptr);
  s->loop.cond = cond;
  return s;
}

Stmt *mkFor(CExpr *cond, Stmt *inc, Stmt *body) {
  Stmt* s         = Stmt::create(FOR, inc, body);
  s->forLoop.cond = cond;
  return s;
}

Stmt *mkPrint(PrintTag t, Expr::Ptr e) {
  Stmt *s      = Stmt::create(PRINT, e, nullptr);
  s->print.tag = t;
  return s;
}

}  // namespace V3DLib
