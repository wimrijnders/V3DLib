//
// This module defines the abstract syntax of the QPU language.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_SOURCE_SYNTAX_H_
#define _V3DLIB_SOURCE_SYNTAX_H_
#include "Common/Stack.h"
#include "Expr.h"
#include "Support/InstructionComment.h"

namespace V3DLib {

// Direction for VPM/DMA loads and stores
enum Dir { HORIZ, VERT };

// Reserved general-purpose vars
enum ReservedVarId : VarId {
  RSV_QPU_ID   = 0,
  RSV_NUM_QPUS = 1
};


// ============================================================================
// Comparison operators
// ============================================================================

// Comparison operators
enum CmpOpId { EQ, NEQ, LT, GT, LE, GE };

// Pair containing comparison operator and base type
struct CmpOp {
	CmpOpId op;
	BaseType type;

	CmpOp(CmpOpId in_op, BaseType in_type) : op(in_op), type(in_type) {}
};


// ============================================================================
// Boolean expressions
// ============================================================================

// Kinds of boolean expressions
enum BExprTag { NOT, AND, OR, CMP };

struct BExpr {
	BExpr() {}
	//BExpr(BExpr const &rhs); 
	BExpr(Expr::Ptr lhs, CmpOp op, Expr::Ptr rhs);

	BExprTag tag() const { return m_tag; }
  Expr::Ptr cmp_lhs();
  Expr::Ptr cmp_rhs();
  void cmp_lhs(Expr::Ptr p);
  void cmp_rhs(Expr::Ptr p);

	BExpr *Not() const;
	BExpr *And(BExpr *rhs) const;
	BExpr *Or(BExpr *rhs) const;

  union {
    // Negation
    BExpr* neg;

    // Conjunction
    struct { BExpr* lhs; BExpr* rhs; } conj;

    // Disjunction
    struct { BExpr* lhs; BExpr* rhs; } disj;

    // Comparison
    struct {
			CmpOp op;
		} cmp;
  };

private:
  BExprTag m_tag;
  Expr::Ptr m_cmp_lhs;  // For comparison
	Expr::Ptr m_cmp_rhs;  // idem
};


// ============================================================================
// Conditional expressions
// ============================================================================

// Kinds of conditional expressions
enum CExprTag { ALL, ANY };

struct CExpr {
	CExpr() {}

  // What kind of boolean expression is it?
  CExprTag tag;

  // This is either a scalar boolean expression, or a reduction of a vector
  // boolean expressions using 'any' or 'all' operators.
  BExpr* bexpr;
};

// Functions to construct conditional expressions
CExpr* mkCExpr();
CExpr* mkAll(BExpr* bexpr);
CExpr* mkAny(BExpr* bexpr);

// ============================================================================
// 'print' statements
// ============================================================================

// For displaying values in emulation
enum PrintTag { PRINT_INT, PRINT_FLOAT, PRINT_STR };

struct PrintStmt {
	PrintTag tag() const { return m_tag;}

	Expr::Ptr expr() const {
		assert((m_tag == PRINT_INT || m_tag == PRINT_FLOAT) && m_expr.get() != nullptr);
		return m_expr;
	}

	char const *str() const { assert(m_tag == PRINT_STR && m_str != nullptr); return m_str; }

private:
  PrintTag    m_tag;
	const char *m_str;
	Expr::Ptr     m_expr;
};

// ============================================================================
// Statements
// ============================================================================

// What kind of statement is it?
enum StmtTag {
	SKIP,
	ASSIGN,
	SEQ,
	WHERE,
	IF,
	WHILE,
	PRINT,
	FOR,
	SET_READ_STRIDE,
	SET_WRITE_STRIDE,
	LOAD_RECEIVE,
	STORE_REQUEST,
	SEND_IRQ_TO_HOST,
	SEMA_INC, SEMA_DEC,
	SETUP_VPM_READ,
	SETUP_VPM_WRITE,
	SETUP_DMA_READ,
	SETUP_DMA_WRITE,
	DMA_READ_WAIT,
	DMA_WRITE_WAIT,
	DMA_START_READ,
	DMA_START_WRITE
};


struct Stmt : public InstructionComment {
	~Stmt();

	static Stmt *create(StmtTag in_tag);
	static Stmt *create(StmtTag in_tag, Expr::Ptr e0, Expr::Ptr e1);
	static Stmt *create(StmtTag in_tag, Stmt *s0, Stmt *s1);

  // What kind of statement is it?
  StmtTag tag;

  union {
    // Assignment
    struct { Expr::Ptr lhs; Expr::Ptr rhs; } assign;

    // Sequential composition
    struct { Stmt* s0; Stmt* s1; } seq;

    // Where
    struct { BExpr* cond; Stmt* thenStmt; Stmt* elseStmt; } where;

    // If
    struct { CExpr* cond; Stmt* thenStmt; Stmt* elseStmt; } ifElse;

    // While
    struct { CExpr* cond; Stmt* body; } loop;

    // For (only used intermediately during AST construction)
    struct { CExpr* cond; Stmt* inc; Stmt* body; } forLoop;

    // Print
    PrintStmt print;

    // Set stride
    Expr::Ptr stride;

    // Load receive destination
    Expr::Ptr loadDest;

    // Store request
    struct { Expr::Ptr data; Expr::Ptr addr; } storeReq;

    // Semaphore id for increment / decrement
    int semaId;

    // VPM read setup
    struct { int numVecs; Expr::Ptr addr; int hor; int stride; } setupVPMRead;

    // VPM write setup
    struct { Expr* addr; int hor; int stride; } setupVPMWrite;

    // DMA read setup
    struct {
			Expr::Ptr vpmAddr;
			int numRows;
			int rowLen;
			int hor;
			int vpitch;
		} setupDMARead;

    // DMA write setup
    struct {
			Expr::Ptr vpmAddr;
			int numRows;
			int rowLen;
			int hor;
		} setupDMAWrite;

    // DMA start read
    Expr::Ptr startDMARead;

    // DMA start write
    Expr::Ptr startDMAWrite;
  };

private:
	void init(StmtTag in_tag);
};


// Functions to construct statements
Stmt *mkSkip();
Stmt *mkAssign(Expr::Ptr lhs, Expr::Ptr rhs);
Stmt *mkSeq(Stmt *s0, Stmt *s1);
Stmt *mkWhere(BExpr *cond, Stmt *thenStmt, Stmt *elseStmt);
Stmt *mkIf(CExpr *cond, Stmt *thenStmt, Stmt *elseStmt);
Stmt *mkWhile(CExpr *cond, Stmt *body);
Stmt *mkFor(CExpr *cond, Stmt *inc, Stmt *body);
Stmt *mkPrint(PrintTag t, Expr::Ptr e);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_SYNTAX_H_
