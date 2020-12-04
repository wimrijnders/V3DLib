#include "Stmt.h"
#include "Syntax.h"
#include "Support/basics.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

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


/**
 * NOTE: The expr for int and float is stored external to this class, in `Stmt`
 *       For a full representation, this needs to be added in stmt::dsp()`
 */
std::string PrintStmt::disp() const {
	std::string ret;

	switch (m_tag) {
	case PRINT_INT:
		ret << "Print Int";
	break;
	case PRINT_FLOAT:
		ret << "Print Float";
	break;
	case PRINT_STR:
		ret << "Print String";
		if (m_str == nullptr) {
			ret << " <nullptr>";
		} else {
			ret << " '" << m_str << "'";
		}
	break;
	default:
		assert(false);
	}

	return ret;
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


Expr::Ptr Stmt::assign_lhs() const {
	assert(tag == ASSIGN);
	assert(m_exp_a.get() != nullptr);
	return m_exp_a;
}


Expr::Ptr Stmt::assign_rhs() const {
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


Expr::Ptr Stmt::print_expr() const {
	assert(tag == PRINT);
	assert(print.tag() == PRINT_INT || print.tag() == PRINT_FLOAT);
	assert(m_exp_a.get() != nullptr);
	assert(m_exp_b.get() == nullptr);
	return m_exp_a;
}


/**
 * Debug routine for easier display of instance contents during debugging
 *
 * Not filled out completely yet, will do that as needed.
 */
std::string Stmt::disp() const {
	std::string ret;

	switch (tag) {
		case SKIP:
			ret << "SKIP";
		break;
		case ASSIGN:
			ret << "ASSIGN " << assign_lhs()->disp() << " = " << assign_rhs()->disp();
		break;
		case SEQ:
			assert(seq.s0 != nullptr);
			assert(seq.s1 != nullptr);
			ret << "SEQ {" << seq.s0->disp() << "; " << seq.s1->disp() << "}";
		break;
		case WHERE:
			assert(where.cond != nullptr);
			assert(where.thenStmt != nullptr);
			ret << "WHERE (" << where.cond->disp() << ") THEN " << where.thenStmt->disp();
			if (where.elseStmt != nullptr) {
				ret << " ELSE " << where.elseStmt->disp();
			}
		break;
		case IF:
			ret << "IF";
		break;
		case WHILE:
			ret << "WHILE";
		break;
		case PRINT:
			ret << print.disp();
			if (print.tag() != PRINT_STR) {
				if (m_exp_a.get() == nullptr) {
					ret << " <no expr>";
				} else {
					ret << m_exp_a->disp();
				}
			}
		break;
		case FOR:
			ret << "FOR";
		break;
		case SET_READ_STRIDE:
			ret << "SET_READ_STRIDE";
		break;
		case SET_WRITE_STRIDE:
			ret << "SET_WRITE_STRIDE";
		break;
		case LOAD_RECEIVE:
			ret << "LOAD_RECEIVE";
		break;
		case STORE_REQUEST:
			ret << "STORE_REQUEST";
		break;
		case SEND_IRQ_TO_HOST:
			ret << "SEND_IRQ_TO_HOST";
		break;
		case SEMA_INC:
			ret << "SEMA_INC";
		break;
		case SEMA_DEC:
			ret << "SEMA_DEC";
		break;
		case SETUP_VPM_READ:
			ret << "SETUP_VPM_READ";
		break;
		case SETUP_VPM_WRITE:
			ret << "SETUP_VPM_WRITE";
		break;
		case SETUP_DMA_READ:
			ret << "SETUP_DMA_READ";
		break;
		case SETUP_DMA_WRITE:
			ret << "SETUP_DMA_WRITE";
		break;
		case DMA_READ_WAIT:
			ret << "DMA_READ_WAIT";
		break;
		case DMA_WRITE_WAIT:
			ret << "DMA_WRITE_WAIT";
		break;
		case DMA_START_READ:
			ret << "DMA_START_READ";
		break;
		case DMA_START_WRITE:
			ret << "DMA_START_WRITE";
		break;
		default:
			assert(false);
		break;
	}

	return ret;
}


Stmt::~Stmt() {
	// WRI DEBUG
	breakpoint
}


Stmt *Stmt::create(StmtTag in_tag) {
	Stmt *ret = new Stmt();
	ret->init(in_tag);
/*

	// WRI debug
	// break for the tags we haven't checked yet
	switch (in_tag) {
		case SKIP:
			break;
		default:
			breakpoint;
			break;
	}
*/

	return ret;
}


Stmt *Stmt::create(StmtTag in_tag, Expr::Ptr e0, Expr::Ptr e1) {
/*
	// WRI debug
	// break for the tags we haven't checked yet
	switch (in_tag) {
		case ASSIGN:
			break;
		default:
			breakpoint;
			break;
	}
*/

	Stmt *ret = new Stmt();
	ret->init(in_tag);

	switch (in_tag) {
		case ASSIGN:
		case STORE_REQUEST:
			assertq(e0 != nullptr && e1 != nullptr, "create 1");
			ret->m_exp_a = e0;
			ret->m_exp_b = e1;
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
			assertq(e0 != nullptr && e1 == nullptr, "create 2");
  		ret->m_exp_a = e0;
		break;
		default:
			fatal("This tag not handled yet in create(Expr,Expr)");
		break;
	}

	return ret;
}


Stmt *Stmt::create(StmtTag in_tag, Stmt* s0, Stmt* s1) {
	Stmt *ret = new Stmt();
	ret->init(in_tag);

	switch (in_tag) {
		case SEQ:
			assertq(s0 != nullptr && s1 != nullptr, "create 3");
  		ret->seq.s0 = s0;
		  ret->seq.s1 = s1;
		break;
		case WHERE:
			// s0, s1 can be nullptr's
			ret->where.cond     = nullptr;  // NOTE: needs to be set elsewhere
			ret->where.thenStmt = s0;
			ret->where.elseStmt = s1;
		break;
		case IF:
			// s0, s1 can be nullptr's
			ret->ifElse.cond     = nullptr;  // NOTE: needs to be set elsewhere
			ret->ifElse.thenStmt = s0;
			ret->ifElse.elseStmt = s1;
		break;
		case WHILE:
			// s0, s1 can be nullptr's
			ret->loop.cond = nullptr;  // NOTE: needs to be set elsewhere
			ret->loop.body = s0;
		break;
		case FOR:
			// s0, s1 can be nullptr's
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
