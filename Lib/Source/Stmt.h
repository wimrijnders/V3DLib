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
	void tag(PrintTag tag) { m_tag = tag;}

	char const *str() const;
	void str(char const *str);

private:
  PrintTag    m_tag;
	const char *m_str;
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
	SEMA_INC,
	SEMA_DEC,
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

	Expr::Ptr assign_lhs() const;
	Expr::Ptr assign_rhs() const;
	Expr::Ptr stride();
	Expr::Ptr loadDest();
	Expr::Ptr storeReq_data();
	Expr::Ptr storeReq_addr();
	Expr::Ptr setupVPMRead_addr();
	Expr::Ptr setupVPMWrite_addr();
	Expr::Ptr setupDMARead_vpmAddr();
	Expr::Ptr setupDMAWrite_vpmAddr();
	Expr::Ptr startDMARead();
	Expr::Ptr startDMAWrite();
	Expr::Ptr print_expr();

	std::string disp() const;

	static Stmt *create(StmtTag in_tag);
	static Stmt *create(StmtTag in_tag, Expr::Ptr e0, Expr::Ptr e1);
	static Stmt *create(StmtTag in_tag, Stmt *s0, Stmt *s1);

  // What kind of statement is it?
  StmtTag tag;

  union {
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

    // Semaphore id for increment / decrement
    int semaId;

    // VPM read setup
    struct { int numVecs; int hor; int stride; } setupVPMRead;

    // VPM write setup
    struct { int hor; int stride; } setupVPMWrite;

    // DMA read setup
    struct {
			int numRows;
			int rowLen;
			int hor;
			int vpitch;
		} setupDMARead;

    // DMA write setup
    struct {
			int numRows;
			int rowLen;
			int hor;
		} setupDMAWrite;
  };

private:
	void init(StmtTag in_tag);

	Expr::Ptr m_exp_a;
	Expr::Ptr m_exp_b;
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
