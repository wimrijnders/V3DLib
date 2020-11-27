//
// This module defines the abstract syntax of the QPU language.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_SOURCE_SYNTAX_H_
#define _V3DLIB_SOURCE_SYNTAX_H_
#include "Common/Heap.h"
#include "Common/Stack.h"
#include "Var.h"
#include "Support/InstructionComment.h"  // for Stmt

namespace V3DLib {

// ============================================================================
// Operators
// ============================================================================

	// Order of operators is important to the random generator, see `Gen.cpp`.
// This applies to the range ROTATE...ROR.
enum OpId {
	// Int & Float operators:
	ROTATE, ADD, SUB, MUL, MIN, MAX,

	// Int only operators:
	SHL, SHR, USHR, BOR, BAND, BXOR, BNOT, ROR,

	// Conversion operators:
	ItoF, FtoI,

	// v3d only
	TIDX,
	EIDX
};


// Every operator has a type associated with it
enum BaseType { UINT8, INT16, INT32, FLOAT };

// Pair containing operator and base type
struct Op {
	OpId op;
	BaseType type;

	Op(OpId in_op, BaseType in_type) : op(in_op), type(in_type) {}

	const char *to_string();
	bool noParams();  // Yes, I know, doesn't make sense. Happens anyway
	bool isUnary();
};


// Is operator commutative?
bool isCommutative(Op op);

// Direction for VPM/DMA loads and stores
enum Dir { HORIZ, VERT };

// Reserved general-purpose vars
enum ReservedVarId : VarId {
  RSV_QPU_ID   = 0,
  RSV_NUM_QPUS = 1
};

// ============================================================================
// Expressions    
// ============================================================================

enum ExprTag {
	INT_LIT,
	FLOAT_LIT,
	VAR,
	APPLY,
	DEREF
};


struct Expr {
	Expr();
	Expr(Var in_var);
	Expr(int in_lit);
	Expr(float in_lit);
	Expr(Expr* lhs, Op op, Expr* rhs);
	Expr(Expr* ptr);

	~Expr();

	ExprTag tag() const { return m_tag; }
	bool isLit() const { return (tag() == INT_LIT) || (tag() == FLOAT_LIT); }

  union {
    int   intLit;                                   // Integer literal
    float floatLit;                                 // Float literal
    Var   var;                                      // Variable identifier

    struct { Expr* lhs; Op op; Expr* rhs; } apply;  // Application of a binary operator
    struct { Expr* ptr; } deref;                    // Dereference a pointer
  };

	bool isSimple() const;

private:
  ExprTag m_tag;  // What kind of expression is it?
};


class BaseExpr {
public:
	BaseExpr() {}
	BaseExpr(Expr *e);
	~BaseExpr();

	Expr *expr() const { return m_expr; }

protected:
  Expr *m_expr = nullptr;  // Abstract syntax tree
};


// Functions to construct expressions
Expr* mkIntLit(int lit);
Expr* mkFloatLit(float lit);
Expr* mkVar(Var var);
Expr* mkApply(Expr* lhs, Op op, Expr* rhs);
Expr* mkDeref(Expr* ptr);


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

  // What kind of boolean expression is it?
  BExprTag tag;

  union {
    // Negation
    BExpr* neg;

    // Conjunction
    struct { BExpr* lhs; BExpr* rhs; } conj;

    // Disjunction
    struct { BExpr* lhs; BExpr* rhs; } disj;

    // Comparison
    struct { Expr* lhs; CmpOp op; Expr* rhs; } cmp;
  };
};

// Functions to construct boolean expressions
BExpr* mkBExpr();
BExpr* mkNot(BExpr* neg);
BExpr* mkAnd(BExpr* lhs, BExpr* rhs);
BExpr* mkOr (BExpr* lhs, BExpr* rhs);
BExpr* mkCmp(Expr*  lhs, CmpOp op, Expr*  rhs);

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
  PrintTag tag;
  union {
    const char* str;
    Expr* expr;
  };
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
	static Stmt *create(StmtTag in_tag, Expr *e0, Expr *e1);
	static Stmt *create(StmtTag in_tag, Stmt *s0, Stmt *s1);

  // What kind of statement is it?
  StmtTag tag;

  union {
    // Assignment
    struct { Expr* lhs; Expr* rhs; } assign;

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
    Expr* stride;

    // Load receive destination
    Expr* loadDest;

    // Store request
    struct { Expr* data; Expr* addr; } storeReq;

    // Semaphore id for increment / decrement
    int semaId;

    // VPM read setup
    struct { int numVecs; Expr* addr; int hor; int stride; } setupVPMRead;

    // VPM write setup
    struct { Expr* addr; int hor; int stride; } setupVPMWrite;

    // DMA read setup
    struct { Expr* vpmAddr; int numRows; int rowLen;
             int hor; int vpitch; } setupDMARead;

    // DMA write setup
    struct { Expr* vpmAddr; int numRows; int rowLen; int hor; } setupDMAWrite;

    // DMA start read
    Expr* startDMARead;

    // DMA start write
    Expr* startDMAWrite;
  };

private:
	void init(StmtTag in_tag);
};

// Functions to construct statements
Stmt* mkSkip();
Stmt* mkAssign(Expr* lhs, Expr* rhs);
Stmt* mkSeq(Stmt* s0, Stmt* s1);
Stmt* mkWhere(BExpr* cond, Stmt* thenStmt, Stmt* elseStmt);
Stmt* mkIf(CExpr* cond, Stmt* thenStmt, Stmt* elseStmt);
Stmt* mkWhile(CExpr* cond, Stmt* body);
Stmt* mkFor(CExpr* cond, Stmt* inc, Stmt* body);
Stmt* mkPrint(PrintTag t, Expr* e);

// ============================================================================
// Global variables
// ============================================================================

//extern Heap astHeap;  // Used for constructing abstract syntax trees

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_SYNTAX_H_
