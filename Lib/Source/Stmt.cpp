#include "Stmt.h"
#include "Syntax.h"
#include "Support/basics.h"

namespace V3DLib {

// ============================================================================
// Class PrintStmt
// ============================================================================

char const *PrintStmt::str() const {
	assert(m_tag == PRINT_STR && m_str != nullptr);
	return m_str;
}


void PrintStmt::str(char const *str) {
	assert(m_str == nullptr);
	m_tag = PRINT_STR;
	m_str = str;
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


Expr::Ptr Stmt::assign_lhs() {
	assert(tag == ASSIGN);
	assert(m_exp_a.get() != nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::assign_rhs() {
	assert(tag == ASSIGN);
	assert(m_exp_b.get() != nullptr);
	return m_exp_b;
}


Expr::Ptr Stmt::stride() {
	assert(tag == SET_READ_STRIDE || tag == SET_WRITE_STRIDE);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::loadDest() {
	assert(tag == LOAD_RECEIVE);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::storeReq_data() {
	assert(tag == STORE_REQUEST);
	assert(m_exp_a.get() != nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::storeReq_addr() {
	assert(tag == STORE_REQUEST);
	assert(m_exp_b.get() != nullptr);
	return m_exp_b;
}


Expr::Ptr Stmt::setupVPMRead_addr() {
	assert(tag == SETUP_VPM_READ);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::setupVPMWrite_addr() {
	assert(tag == SETUP_VPM_WRITE);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::setupDMARead_vpmAddr() {
	assert(tag == SETUP_DMA_READ);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::setupDMAWrite_vpmAddr() {
	assert(tag == SETUP_DMA_WRITE);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::startDMARead() {
	assert(tag == DMA_START_READ);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::startDMAWrite() {
	assert(tag == DMA_START_WRITE);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::print_expr() {
	assert(tag == PRINT);
	assert(print.tag() == PRINT_INT || print.tag() == PRINT_FLOAT);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
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
		case STORE_REQUEST:
			assertq(e0 != nullptr && e1 != nullptr, "");
			ret->m_exp_a = e0;
			ret->m_exp_b = e0;
		break;

		case PRINT:
		case LOAD_RECEIVE:
		case SET_READ_STRIDE:
		case SET_WRITE_STRIDE:
		case SETUP_VPM_READ:
		case SETUP_VPM_WRITE:
		case SETUP_DMA_READ:
		case SETUP_DMA_WRITE:
		case DMA_START_READ:
		case DMA_START_WRITE:
			assertq(e0 != nullptr && e1 == nullptr, "");
  		ret->m_exp_a = e0;
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
  Stmt *s = Stmt::create(PRINT, e, nullptr);
  s->print.tag(t);
  return s;
}



}  // namespace V3DLib
