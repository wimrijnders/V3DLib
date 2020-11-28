#ifndef _V3DLIB_SOURCE_STMT_H_
#define _V3DLIB_SOURCE_STMT_H_
#include "Support/InstructionComment.h"
#include "Int.h"
#include "Expr.h"

namespace V3DLib {

class BExpr;
class CExpr;

// ============================================================================
// Class PrintStmt
// ============================================================================

// For displaying values in emulation
enum PrintTag { PRINT_INT, PRINT_FLOAT, PRINT_STR };

struct PrintStmt {
	PrintTag tag() const { return m_tag;}

	Expr::Ptr expr() const;
	char const *str() const;
	void str(char const *str);
	void expr(IntExpr x);

private:
  PrintTag    m_tag;
	const char *m_str;
	Expr::Ptr   m_expr;
};


// ============================================================================
// Class Stmt
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

#endif  // _V3DLIB_SOURCE_STMT_H_
