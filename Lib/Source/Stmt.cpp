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


Stmt::Ptr Stmt::seq_s0() const {
 	assert(tag == SEQ);
	assert(m_stmt_a.get() != nullptr);
	return m_stmt_a;
}


Stmt::Ptr Stmt::seq_s1() const {
 	assert(tag == SEQ);
	assert(m_stmt_b.get() != nullptr);
	return m_stmt_b;
}


Stmt::Ptr Stmt::thenStmt() const {
 	assertq(tag == IF || tag == WHERE, "Then-statement only valid for IF and WHERE", true);
	// where and else stmt may not both be null
	assert(m_stmt_a.get() != nullptr || m_stmt_b.get() != nullptr);
	return m_stmt_a;  // May be null
}


Stmt::Ptr Stmt::body() const {
 	assertq(tag == WHILE || tag == FOR, "Body-statement only valid for WHILE and FOR", true);
	assert(m_stmt_a.get() != nullptr);
	return m_stmt_a;
}


Stmt::Ptr Stmt::elseStmt() const {
 	assertq(tag == IF || tag == WHERE, "Else-statement only valid for IF and WHERE", true);
	// where and else stmt may not both be null
	assert(m_stmt_a.get() != nullptr || m_stmt_b.get() != nullptr);
	return m_stmt_b; // May be null
}


void Stmt::thenStmt(Ptr then_ptr) {
 	assertq(tag == IF || tag == WHERE, "Then-statement only valid for IF and WHERE", true);
	assert(m_stmt_a.get() == nullptr);  // Only assign once
	m_stmt_a = then_ptr;
}


void Stmt::elseStmt(Ptr else_ptr) {
 	assertq(tag == IF || tag == WHERE, "Else-statement only valid for IF and WHERE", true);
	assert(m_stmt_b.get() == nullptr);  // Only assign once
	m_stmt_b = else_ptr;
}


void Stmt::body(Ptr ptr) {
 	assertq(tag == WHILE || tag == FOR, "Body-statement only valid for WHILE and FOR", true);
	assert(m_stmt_a.get() == nullptr);  // Only assign once
	m_stmt_a = ptr;
}


void Stmt::inc(Ptr ptr) {
 	assertq(tag == FOR, "Inc-statement only valid for FOR", true);
	assert(m_stmt_b.get() == nullptr);  // Only assign once
	m_stmt_b = ptr;
}


bool Stmt::then_is_null() const {
 	assertq(tag == IF || tag == WHERE, "Then-statement only valid for IF and WHERE", true);
	return m_stmt_a.get() == nullptr;
}


bool Stmt::else_is_null() const {
 	assertq(tag == IF || tag == WHERE, "Else-statement only valid for IF and WHERE", true);
	return m_stmt_b.get() == nullptr;
}


bool Stmt::body_is_null() const {
 	assertq(tag == WHILE || tag == FOR, "Body-statement only valid for WHILE and FOR", true);
	return m_stmt_a.get() == nullptr;
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
			assert(seq_s0().get() != nullptr);
			assert(seq_s1().get() != nullptr);
			ret << "SEQ {" << seq_s0()->disp() << "; " << seq_s1()->disp() << "}";
		break;
		case WHERE:
			assert(where.cond != nullptr);
			assert(thenStmt().get() != nullptr);
			ret << "WHERE (" << where.cond->disp() << ") THEN " << thenStmt()->disp();
			if (elseStmt().get() != nullptr) {
				ret << " ELSE " << elseStmt()->disp();
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

		case FOR:              ret << "FOR";              break;
		case SET_READ_STRIDE:  ret << "SET_READ_STRIDE";  break;
		case SET_WRITE_STRIDE: ret << "SET_WRITE_STRIDE"; break;
		case LOAD_RECEIVE:     ret << "LOAD_RECEIVE";     break;
		case STORE_REQUEST:    ret << "STORE_REQUEST";    break;
		case SEND_IRQ_TO_HOST: ret << "SEND_IRQ_TO_HOST"; break;
		case SEMA_INC:         ret << "SEMA_INC";         break;
		case SEMA_DEC:         ret << "SEMA_DEC";         break;
		case SETUP_VPM_READ:   ret << "SETUP_VPM_READ";   break;
		case SETUP_VPM_WRITE:  ret << "SETUP_VPM_WRITE";  break;
		case SETUP_DMA_READ:   ret << "SETUP_DMA_READ";   break;
		case SETUP_DMA_WRITE:  ret << "SETUP_DMA_WRITE";  break;
		case DMA_READ_WAIT:    ret << "DMA_READ_WAIT";    break;
		case DMA_WRITE_WAIT:   ret << "DMA_WRITE_WAIT";   break;
		case DMA_START_READ:   ret << "DMA_START_READ";   break;
		case DMA_START_WRITE:  ret << "DMA_START_WRITE";  break;

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


Stmt::Ptr Stmt::create(StmtTag in_tag) {
	Ptr ret(new Stmt());
	ret->init(in_tag);
	return ret;
}


Stmt::Ptr Stmt::create(StmtTag in_tag, Expr::Ptr e0, Expr::Ptr e1) {
	Ptr ret(new Stmt());
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


Stmt::Ptr Stmt::create(StmtTag in_tag, Ptr s0, Ptr s1) {
	Ptr ret(new Stmt());
	ret->init(in_tag);

	switch (in_tag) {
		case SEQ:
			assertq(s0.get() != nullptr && s1.get() != nullptr, "create 3");
			assertq(ret->m_stmt_a.get() == nullptr, "create() SEQ: don't reassign stmt a ptr");
			assertq(ret->m_stmt_b.get() == nullptr, "create() SEQ: don't reassign stmt b ptr");

  		ret->m_stmt_a = s0;
  		ret->m_stmt_b = s1;
		break;
		case WHERE:
			// s0, s1 can be nullptr's
			assertq(ret->m_stmt_a.get() == nullptr, "create() WHERE: don't reassign stmt a ptr");
			assertq(ret->m_stmt_b.get() == nullptr, "create() WHERE: don't reassign stmt b ptr");

			ret->where.cond = nullptr;  // NOTE: needs to be set elsewhere
  		ret->m_stmt_a = s0;
  		ret->m_stmt_b = s1;
		break;
		case IF:
			// s0, s1 can be nullptr's
			assertq(ret->m_stmt_a.get() == nullptr, "create() IF: don't reassign stmt a ptr");
			assertq(ret->m_stmt_b.get() == nullptr, "create() IF: don't reassign stmt b ptr");

			ret->ifElse.cond = nullptr;  // NOTE: needs to be set elsewhere
  		ret->m_stmt_a = s0;
  		ret->m_stmt_b = s1;
		break;
		case WHILE:
			// s0 can be nullptr, s1 not used
			assertq(ret->m_stmt_a.get() == nullptr, "create() WHILE: don't reassign stmt a ptr");
			assertq(ret->m_stmt_b.get() == nullptr, "create() WHILE: don't reassign stmt b ptr");
			assertq(s1.get() == nullptr, "create() WHILE: not expecting assignment stmt b ptr");

			ret->loop.cond = nullptr;  // NOTE: needs to be set elsewhere
  		ret->m_stmt_a = s0;
		break;
		case FOR:
			// s0, s1 can be nullptr's
 			// m_stmt_a is body, m_stmt_b is inc.
			assertq(false, "create(): Not really expecting FOR-tag", true);
			assertq(ret->m_stmt_a.get() == nullptr, "create() FOR: don't reassign stmt a ptr");
			assertq(ret->m_stmt_b.get() == nullptr, "create() FOR: don't reassign stmt b ptr");

			ret->forLoop.cond = nullptr;  // NOTE: needs to be set elsewhere
  		ret->m_stmt_a = s1;  // NOTE: s1, s0 reversed
  		ret->m_stmt_b = s0;
		break;
		default:
			fatal("This tag not handled yet in create(Stmt,Stmt)");
		break;
	}

	return ret;
}


Stmt::Ptr Stmt::create_assign(Expr::Ptr lhs, Expr::Ptr rhs) {
	return create(ASSIGN, lhs, rhs);
}


Stmt::Ptr Stmt::create_sequence(Ptr s0, Ptr s1) {
	return Stmt::create(SEQ, s0, s1);
}


/**
 * Convert 'for' loop to 'while' loop
 *
 * m_stmt_a is body, m_stmt_b is inc.
 */
void Stmt::for_to_while(Ptr in_body) {
	assertq(tag == FOR, "Only FOR-statement can be converted to WHILE", true);
	assert(m_stmt_a.get() == nullptr);  // Don't reassign body

	auto inc = m_stmt_b;  // TODO Check should inc be reset to null here?

	CExpr* whileCond = forLoop.cond;
	Stmt::Ptr whileBody = Stmt::create_sequence(in_body, inc);
	tag = WHILE;
	body(whileBody);
	loop.cond = whileCond;
}


// ============================================================================
// Functions on statements
// ============================================================================

Stmt::Ptr mkSkip() { return Stmt::create(SKIP); }

Stmt::Ptr mkWhere(BExpr *cond, Stmt::Ptr thenStmt, Stmt::Ptr elseStmt) {
  Stmt::Ptr s   = Stmt::create(WHERE, thenStmt, elseStmt);
  s->where.cond = cond;
  return s;
}


Stmt::Ptr mkIf(CExpr *cond, Stmt *thenStmt, Stmt *elseStmt) {
	if (thenStmt != nullptr || elseStmt != nullptr) {
		breakpoint  // Deal with this case when it happens
	}

  Stmt::Ptr s    = Stmt::create(IF, Stmt::Ptr(thenStmt), Stmt::Ptr(elseStmt));
  s->ifElse.cond = cond;
  return s;
}


Stmt::Ptr mkWhile(CExpr *cond, Stmt::Ptr body) {
  Stmt::Ptr s  = Stmt::create(WHILE, body, Stmt::Ptr());
  s->loop.cond = cond;
  return s;
}


Stmt::Ptr mkFor(CExpr *cond, Stmt::Ptr inc, Stmt::Ptr body) {
  Stmt::Ptr s     = Stmt::create(FOR, inc, body);
  s->forLoop.cond = cond;
  return s;
}


Stmt::Ptr mkPrint(PrintTag t, Expr::Ptr e) {
  Stmt::Ptr s = Stmt::create(PRINT, e, nullptr);
  s->print.tag(t);
  return s;
}

}  // namespace V3DLib
