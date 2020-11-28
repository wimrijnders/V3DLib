#include "Stmt.h"
#include "Syntax.h"
#include "Support/basics.h"

namespace V3DLib {

// ============================================================================
// Class PrintStmt
// ============================================================================

Expr::Ptr PrintStmt::expr() const {
	assert((m_tag == PRINT_INT || m_tag == PRINT_FLOAT) && m_expr.get() != nullptr);
	return m_expr;
}


char const *PrintStmt::str() const { assert(m_tag == PRINT_STR && m_str != nullptr); return m_str; }


void PrintStmt::str(char const *str) {
	assert(m_str == nullptr);
	assert(m_expr.get() == nullptr);
	m_tag = PRINT_STR;
	m_str = str;
}


void PrintStmt::expr(IntExpr x) {
	assert(m_str == nullptr);
	assert(m_expr.get() == nullptr);
	m_tag = PRINT_INT;
	m_expr = x.expr();
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
