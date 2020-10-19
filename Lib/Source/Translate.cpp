#include "Translate.h"
#include "../SourceTranslate.h"
#include "Source/Syntax.h"
#include "Target/SmallLiteral.h"
#include "Common/Seq.h"


namespace QPULib {
namespace {

// ============================================================================
// Opcodes and operands
// ============================================================================

// Translate source operator to target opcode
ALUOp opcode(Op op)
{
  if (op.type == FLOAT) {
    switch (op.op) {
      case ADD:    return A_FADD;
      case SUB:    return A_FSUB;
      case MUL:    return M_FMUL;
      case MIN:    return A_FMIN;
      case MAX:    return A_FMAX;
      case ItoF:   return A_ItoF;
      case ROTATE: return M_ROTATE;
      default:     assert(false);
    }
  }
  else {
    switch (op.op) {
      case ADD:    return A_ADD;
      case SUB:    return A_SUB;
      case MUL:    return M_MUL24;
      case MIN:    return A_MIN;
      case MAX:    return A_MAX;
      case FtoI:   return A_FtoI;
      case SHL:    return A_SHL;
      case SHR:    return A_ASR;
      case USHR:   return A_SHR;
      case ROR:    return A_ROR;
      case BAND:   return A_BAND;
      case BOR:    return A_BOR;
      case BXOR:   return A_BXOR;
      case BNOT:   return A_BNOT;
      case ROTATE: return M_ROTATE;
      case TIDX: 
				assertq(!compiling_for_vc4(), "opcode(): TIDX is only for v3d", true);
				return A_TIDX;
      case EIDX: 
				assertq(!compiling_for_vc4(), "opcode(): EIDX is only for v3d", true);
				return A_EIDX;
      default:
				assertq(false, "Not expecting this op for int in opcode()", true);
				break;
    }
  }

	return NOP;
}


// Translate the argument of an operator (either a variable or a small
// literal) to a target operand.

RegOrImm operand(Expr* e)
{
  RegOrImm x;
  if (e->tag == VAR) {
    x.tag = REG;
    x.reg = srcReg(e->var);
    return x;
  }
  int enc = encodeSmallLit(e);
  assert(enc >= 0);
  x.tag          = IMM;
  x.smallImm.tag = SMALL_IMM;
  x.smallImm.val = enc;
  return x;
}

// ============================================================================
// 'Simple' expressions
// ============================================================================

// An expression is 'simple' if it is a small literal (see
// Target/SmallLiteral.cpp) or a variable.

bool isSimple(Expr* e)
{
  return (e->tag == VAR) || isSmallLit(e);
}

// Translate an expression to a simple expressions, generating
// instructions along the way.  (Prototype here, implementation below,
// since this and 'varAssign' are mutually recursive.)

Expr* simplify(Seq<Instr>* seq, Expr* e);


// ============================================================================
// Variable assignments
// ============================================================================

/**
 * Translate an expression to a simple expression, generating
 * instructions along the way.
 */
Expr* simplify(Seq<Instr>* seq, Expr* e) {
  if (isSimple(e)) {
		return e;
	}

	Var tmp = freshVar();
	varAssign(seq, tmp, e);
	return mkVar(tmp);
}


// ============================================================================
// Assignment statements
// ============================================================================

/**
 * @param seq      Target instruction sequence to extend
 * @param lhsExpr  Expression on left-hand side
 * @param rhs      Expression on right-hand side
 */
void assign(Seq<Instr>* seq, Expr *lhsExpr, Expr *rhs) {
  Expr lhs = *lhsExpr;

  // -----------------------------------------------------------
  // Case: v := rhs, where v is a variable and rhs an expression
  // -----------------------------------------------------------
  if (lhs.tag == VAR) {
    varAssign(seq, lhs.var, rhs);
    return;
  }

  // ---------------------------------------------------------
  // Case: *lhs := rhs where lhs is not a var or rhs not a var
  // ---------------------------------------------------------
  if (lhs.tag == DEREF && (lhs.deref.ptr->tag != VAR || rhs->tag != VAR)) {
    assert(!isLit(lhs.deref.ptr));
    lhs.deref.ptr = simplify(seq, lhs.deref.ptr);
    rhs = putInVar(seq, rhs);
  }

  // -------------------------------------------------
  // Case: *v := rhs where v is a var and rhs is a var
  // -------------------------------------------------

	// Strictly speaking, the two VAR tests are not necessary; it is the only possible case
	// (According to previous code, that is)
  bool handle_case = (lhs.tag == DEREF && (lhs.deref.ptr->tag == VAR || rhs->tag == VAR));
	if (handle_case) {
		getSourceTranslate().deref_var_var(seq, lhs, rhs);
		return;
	}

  // This case should not be reachable
  assert(false);
}

// ============================================================================
// Condition flag helpers
// ============================================================================

// Each QPU contains an implicit condition vector which can answer
// various questions about each element of a vector:
//
//   * ZS - is zero?
//   * ZC - is non-zero?
//   * NS - is negative?
//   * NC - is non-negative?
//
// The condition vector is modified when the 'setFlags' field of
// an ALU instruction is 'true'.  The condition vector can be read
// from an assignment condition or in a branch condition.


/**
 * Function to negate a branch condition.
 */
BranchCond negBranchCond(BranchCond cond) {
  switch (cond.tag) {
    case COND_NEVER:  cond.tag  = COND_ALWAYS; return cond;
    case COND_ALWAYS: cond.tag  = COND_NEVER;  return cond;
    case COND_ANY:    cond.tag  = COND_ALL;
                      cond.flag = negFlag(cond.flag);
                      return cond;
    case COND_ALL:    cond.tag  = COND_ANY;
                      cond.flag = negFlag(cond.flag);
                      return cond;
  }

  // Not reachable
  assert(false);
	return cond;
}



// Return a value that will cause the specified flag bit to be set in
// the condition vector.

int setFlag(Flag f)
{
  switch (f) {
    case ZS: return 0;
    case ZC: return 1;
    case NS: return -1;
    case NC: return 0;
  }

  // Not reachable
  assert(false);
	return -1;
}


/**
 * Set the condition vector using given variable.
 */
Instr setCond(Var v) {
  Instr instr(ALU);
	instr. ALU.setFlags         = true;
  instr.ALU.dest              = Target::instr::None;
  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg          = srcReg(v);
  instr.ALU.op                = A_BOR;
  instr.ALU.srcB.tag          = REG;
  instr.ALU.srcB.reg          = instr.ALU.srcA.reg;
  return instr;
}


/**
 * A shorthand 'move' instruction is handy later.
 */
Instr move(Var dst, Var src, bool setFlags) {
  Instr instr = Target::instr::mov(dstReg(dst), srcReg(src));
  instr.ALU.setFlags          = setFlags;

  return instr;
}


// ============================================================================
// Boolean expressions
// ============================================================================

// Evaluating a vector boolean expression results in a condition
// pair <condVar,condFlag> where
//
//  * condVar is a variable containing a vector of values
//
//  * condFlag is a condition flag in set {ZS,ZC,NS,NC} (see above)
//
// If 'condVar' is assigned to a register and the 'setFlags' field of
// the assignment is 'true', the implicit condition vector is updated.
// The implicit condition vector can then be queried using 'condFlag'
// to determine the truth of elements in the boolean vector.
//
// For example, assuming vectors of size 4 for simplicity, the result
// of evaluating
//
//   [1,2,3,4] <= [4,1,3,6]
//
// might be the condition pair <[-3,1,0,-2], NC>.
//
// Given two condition pairs <condVarA, condFlagA> and <condVarB,
// condFlagB> we would like to compute the logical disjunction.
//
// Pre-condition: we are in a state where the implicit condition
// vector has been set using the value of condVarB, hence we don't
// need the value of condVarB as an argument.
//
// The 'modify' flag defines whether or not to update the implicit
// condition vector with the final result.
//
// The value of condVarA will be overwritten with the 'condVar' of the
// disjunction, and the corresponding condFlag will be returned as a
// result.

AssignCond boolOr(
	Seq<Instr>* seq,
	AssignCond condA,
	Var condVarA,
	AssignCond condB,
	bool modify
) {
  if (condA.is_always()) return condA;
  else if (condB.is_always()) return condB;
  else if (condB.is_never()) {
    if (modify) *seq << setCond(condVarA);
    return condA;
  }
  else if (condA.is_never()) {
    Instr instr(LI);
    instr.LI.cond       = condB;
    instr.LI.dest       = dstReg(condVarA);
    instr.LI.imm.tag    = IMM_INT32;
    instr.LI.imm.intVal = setFlag(condB.flag);

    *seq << instr;
    return condB;
  }
  else {
    Instr instr(LI);
    instr.LI.cond       = condB;
    instr.LI.dest       = dstReg(condVarA);
    instr.LI.imm.tag    = IMM_INT32;
    instr.LI.imm.intVal = setFlag(condA.flag);

    *seq << instr;
    if (modify) *seq << setCond(condVarA);
    return condA;
  }
}


/**
 * Conjunction is now easy thanks to De Morgan's law
 */
AssignCond boolAnd(
	Seq<Instr>* seq,
	AssignCond condA,
	Var condVarA,
	AssignCond condB,
	bool modify
) {
  return boolOr(seq, condA.negate(), condVarA, condB.negate(), modify).negate();
}


// Now the translation scheme for general boolean expressions.
// The interface is:
// 
//   * a boolean expression to evaluate;
//
//   * a condVar 'v' to which the evaluated expression will be written
//     to; the return value will contain the corresponding condFlag.
//
//   * if the modify-bit is true, then the implicit condition vector
//     will be set using with the result of the expression.  (This is a
//     one-way 'if': you cannot rely on the condition vector not
//     being mutated even if this bit is false.)
//
//   * instructions to evaluate the expression are appended to the
//     given instruction sequence.

AssignCond boolExp( Seq<Instr>* seq , BExpr* bexpr , Var v , bool modify) {
  BExpr b = *bexpr;

  // -------------------------------
  // Case: x > y, replace with y < x
  // -------------------------------
  if (b.tag == CMP && b.cmp.op.op == GT) {
    Expr* e     = b.cmp.lhs;
    b.cmp.lhs   = b.cmp.rhs;
    b.cmp.rhs   = e;
    b.cmp.op.op = LT;
  }

  // ---------------------------------
  // Case: x <= y, replace with y >= x
  // ---------------------------------
  if (b.tag == CMP && b.cmp.op.op == LE) {
    Expr* e     = b.cmp.lhs;
    b.cmp.lhs   = b.cmp.rhs;
    b.cmp.rhs   = e;
    b.cmp.op.op = GE;
  }

  // -----------------------------------
  // Case: x op y, where x is not simple
  // -----------------------------------
  if (b.tag == CMP && !isSimple(b.cmp.lhs)) {
    b.cmp.lhs = simplify(seq, b.cmp.lhs);
  }

  // -----------------------------------
  // Case: x op y, where y is not simple
  // -----------------------------------
  if (b.tag == CMP && !isSimple(b.cmp.rhs)) {
    b.cmp.rhs = simplify(seq, b.cmp.rhs);
  }

  // ---------------------------------------------
  // Case: x op y, where x and y are both literals
  // ---------------------------------------------
  if (b.tag == CMP && isLit(b.cmp.lhs) && isLit(b.cmp.rhs)) {
    Var tmpVar = freshVar();
    varAssign(seq, tmpVar, b.cmp.lhs);
    b.cmp.lhs = mkVar(tmpVar);
  }

  // --------------------------------------
  // Case: x op y, where x and y are simple
  // --------------------------------------
  if (b.tag == CMP) {
    // Compute condition flag
		SetCond set_cond = NO_COND;  // For v3d
    AssignCond cond(AssignCond::Tag::FLAG);

    switch(b.cmp.op.op) {
      case EQ:  cond.flag = ZS; set_cond = Z; break;
      case NEQ: cond.flag = ZC; set_cond = Z; break;
      case LT:  cond.flag = NS; set_cond = N; break;
      case GE:  cond.flag = NC; set_cond = N; break;
      default:  assert(false);
    }

    // Implement comparison using subtraction instruction
    Op op;
    op.type = b.cmp.op.type;
    op.op   = SUB;

    Instr instr(ALU);
    instr.ALU.setFlags = true;
    instr.ALU.setCond  = set_cond;
    instr.ALU.dest     = dstReg(v);
    instr.ALU.srcA     = operand(b.cmp.lhs);
    instr.ALU.op       = opcode(op);
    instr.ALU.srcB     = operand(b.cmp.rhs);
    seq->append(instr);

    return cond;
  }

  // -----------------------------------------
  // Case: !b, where b is a boolean expression
  // -----------------------------------------
  if (b.tag == NOT) {
    AssignCond cond = boolExp(seq, b.neg, v, modify);
    return cond.negate();
  }

  // ------------------------------------------------
  // Case: a || b, where a, b are boolean expressions
  // ------------------------------------------------
  if (b.tag == OR) {
    Var w = freshVar();
    AssignCond condA = boolExp(seq, b.disj.lhs, v, false);
    AssignCond condB = boolExp(seq, b.disj.rhs, w, true);
    return boolOr(seq, condA, v, condB, true);
  }

  // ------------------------------------------------
  // Case: a && b, where a, b are boolean expressions
  // ------------------------------------------------
  if (b.tag == AND) {
    // Use De Morgan's law
    BExpr* demorgan = mkNot(mkOr(mkNot(b.conj.lhs), mkNot(b.conj.rhs)));
    return boolExp(seq, demorgan, v, modify);
  }

  // Not reachable
  assert(false);
	return always;
}


// ============================================================================
// Conditional expressions
// ============================================================================

BranchCond condExp(Seq<Instr>* seq, CExpr* c) {
  Var v = freshVar();
  AssignCond cond = boolExp(seq, c->bexpr, v, true);

  BranchCond bcond;
  if (cond.is_always()) { bcond.tag = COND_ALWAYS; return bcond; }
  if (cond.is_never())  { bcond.tag = COND_NEVER; return bcond; }

  assert(cond.tag == AssignCond::Tag::FLAG);

  bcond.flag = cond.flag;
  if (c->tag == ANY) {
    bcond.tag = COND_ANY;
    return bcond;
  }
  else if (c->tag == ALL) {
    bcond.tag = COND_ALL;
    return bcond;
  }

  // Not reachable
  assert(false);
	return bcond;
}


// ============================================================================
// Where statements
// ============================================================================

void whereStmt(
	Seq<Instr>* seq,
	Stmt* s,
	Var condVar,
	AssignCond cond,
	bool saveRestore
) {
  if (s == NULL) return;

  // ----------
  // Case: skip
  // ----------
  if (s->tag == SKIP) return;

  // ------------------------------------------------------
  // Case: v = e, where v is a variable and e an expression
  // ------------------------------------------------------
  if (s->tag == ASSIGN && s->assign.lhs->tag == VAR) {
    varAssign(seq, cond, s->assign.lhs->var, s->assign.rhs);
    return;
  }

  // ---------------------------------------------
  // Case: s0 ; s1, where s0 and s1 are statements
  // ---------------------------------------------
  if (s->tag == SEQ) {
    whereStmt(seq, s->seq.s0, condVar, cond, true);
    whereStmt(seq, s->seq.s1, condVar, cond, saveRestore);
    return;
  }

  // ----------------------------------------------------------
  // Case: where (b) s0 s1, where b is a boolean expression and
  //                        s0 and s1 are statements.
  // ----------------------------------------------------------
  if (s->tag == WHERE) {
    if (cond.is_always()) {
      // This case has a cheaper implementation

      // Compile new boolean expression
      AssignCond newCond = boolExp(seq, s->where.cond, condVar, true);

      // Compile 'then' statement
      if (s->where.thenStmt != NULL)
        whereStmt(seq, s->where.thenStmt, condVar, newCond,
          s->where.elseStmt != NULL);

      // Compile 'else' statement
      if (s->where.elseStmt != NULL)
        whereStmt(seq, s->where.elseStmt, condVar, newCond.negate(), false);
    }
    else {
      // Save condVar
      Var savedCondVar = freshVar();
      if (saveRestore || s->where.elseStmt != NULL)
        seq->append(move(savedCondVar, condVar, false));

      // Compile new boolean expression
      Var newCondVar = freshVar();
      AssignCond newCond = boolExp(seq, s->where.cond, newCondVar, true);

      if (s->where.thenStmt != NULL) {
        // AND new boolean expression with original condition
        AssignCond andCond = boolAnd(seq, cond, condVar, newCond, true);

        // Compile 'then' statement
        whereStmt(seq, s->where.thenStmt, condVar, andCond, false);
      }

      if (saveRestore || s->where.elseStmt != NULL)
        seq->append(move(condVar, savedCondVar, true));

      if (s->where.elseStmt != NULL) {
        // AND negation of new boolean expression with original condition
        AssignCond andCond = boolAnd(seq, newCond.negate(), newCondVar, cond, true);
  
        // Compile 'else' statement
        whereStmt(seq, s->where.elseStmt, newCondVar, andCond, false);
  
        // Restore condVar and implicit condition vector
        if (saveRestore)
          seq->append(move(condVar, savedCondVar, true));
      }
    }

    return;
  }

  printf("QPULib: only assignments and nested 'where' \
          statements can occur in a 'where' statement\n");
  assert(false);
}


// ============================================================================
// Print statements
// ============================================================================

void printStmt(Seq<Instr>* seq, PrintStmt s) {
  Instr instr(PRI);

  switch (s.tag) {
    case PRINT_INT:
    case PRINT_FLOAT: {
      Var tmpVar = freshVar();
      varAssign(seq, tmpVar, s.expr);

      if (s.tag == PRINT_INT) {
        instr.PRI = srcReg(tmpVar);
      }
      else {
        instr.PRF = srcReg(tmpVar);
      }

      *seq << instr;
      return;
    }
    case PRINT_STR:
      instr.tag = PRS;
      instr.PRS = s.str;
      *seq << instr;
      return;
  }

  assert(false);
}


// ============================================================================
// Load receive statements
// ============================================================================

void loadReceive(Seq<Instr>* seq, Expr* dest) {
  assert(dest->tag == VAR);
  Instr instr(RECV);

  instr.RECV.dest = dstReg(dest->var);
  *seq << instr;
}


// ============================================================================
// Statements
// ============================================================================

void stmt(Seq<Instr>* seq, Stmt* s) {
  if (s == NULL) return;

  // ----------
  // Case: skip
  // ----------
  if (s->tag == SKIP) return;

  // --------------------------------------------------
  // Case: lhs = rhs, where lhs and rhs are expressions
  // --------------------------------------------------
  if (s->tag == ASSIGN) {
    assign(seq, s->assign.lhs, s->assign.rhs);
    return;
  }

  // ---------------------------------------------
  // Case: s0 ; s1, where s1 and s2 are statements
  // ---------------------------------------------

  if (s->tag == SEQ) {
    stmt(seq, s->seq.s0);
    stmt(seq, s->seq.s1);
    return;
  }

  // -------------------------------------------------------------------
  // Case: if (c) s0 s1, where c is a condition, and lhs,rhs expressions
  // -------------------------------------------------------------------
  if (s->tag == IF) {
		using namespace Target::instr;

    Label elseLabel  = freshLabel();
    Label endifLabel = freshLabel();
    BranchCond cond  = condExp(seq, s->ifElse.cond);  // Compile condition
    
		*seq << branch(negBranchCond(cond), elseLabel);   // Branch to 'else' statement
    stmt(seq, s->ifElse.thenStmt);                    // Compile 'then' statement

    if (s->ifElse.elseStmt != NULL) {
			*seq << branch(endifLabel);                     // Branch to endif
		}

		*seq << label(elseLabel);                         // Label for 'else' statement
    stmt(seq, s->ifElse.elseStmt);                    // Compile 'else' statement
		*seq << label(endifLabel);                        // Label for endif

    return;
  }

  // -----------------------------------------------------------
  // Case: while (c) s where c is a condition, and s a statement
  // -----------------------------------------------------------
  if (s->tag == WHILE) {
		using namespace Target::instr;

    Label startLabel = freshLabel();
    Label endLabel   = freshLabel();
    BranchCond cond  = condExp(seq, s->loop.cond);      // Compile condition
 
		*seq << branch(negBranchCond(cond), endLabel)       // Branch over loop body
		     << label(startLabel);                          // Start label

    if (s->loop.body != NULL) stmt(seq, s->loop.body);  // Compile body
    condExp(seq, s->loop.cond);                         // Compute condition again
		                                                    // TODO why is this necessary?

		*seq << branch(cond, startLabel)                    // Branch to start
		     << label(endLabel);                            // End label

    return;
  }

  // ----------------------------------------------------------------------
  // Case: where (b) s0 s1 where c is a boolean expr, and s0, s1 statements
  // ----------------------------------------------------------------------
  if (s->tag == WHERE) {
    Var condVar = freshVar();
    whereStmt(seq, s, condVar, always, false);
    return;
  }

  // ---------------------------------------------
  // Case: print(e) where e is an expr or a string
  // ---------------------------------------------
  if (s->tag == PRINT) {
    printStmt(seq, s->print);
    return;
  }

  // -----------------------------------
  // Case: receive(e) where e is an expr
  // -----------------------------------
  if (s->tag == LOAD_RECEIVE) {
    loadReceive(seq, s->loadDest);
    return;
  }

	// Handle platform-specific instructions
	if (getSourceTranslate().stmt(seq, s)) {
		return;
	}

  // Not reachable
  assert(false);
}


/**
 * Insert 'end' instruction
 */
void insertEndCode(Seq<Instr> &seq) {
  Instr instr;
  instr.tag = END;

	seq << instr;
}


/**
 * Insert markers for initialization code
 *
 * Only used for `v3d`.
 */
void insertInitBlock(Seq<Instr> &code) {
	// Find first instruction after uniform loads
	int index = 0;
	for (; index < code.size(); ++index) {
		if (!code[index].isUniformLoad()) break; 
	}
	assertq(index >= 2, "Expecting at least two uniform loads.");

	Seq<Instr> ret;

  Instr instr;
  instr.tag = INIT_BEGIN;
	ret << instr;
  instr.tag = INIT_END;
	ret << instr;

	code.insert(index, ret);
}

}  // anon namespace


// ============================================================================
// Interface
// ============================================================================

/**
 * Translate variable to source register.
 */
Reg srcReg(Var v) {
  Reg r;
	r.isUniformPtr = false;

  switch (v.tag) {
    case UNIFORM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_UNIFORM;
			r.isUniformPtr = v.isUniformPtr;
			break;
    case QPU_NUM:
			//if (!compiling_for_vc4()) breakpoint
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_QPU_NUM;
			break;
    case ELEM_NUM:
			//if (!compiling_for_vc4()) breakpoint
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_ELEM_NUM;
			break;
    case VPM_READ:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_VPM_READ;
			break;
    case STANDARD:
      r.tag   = REG_A;
      r.regId = v.id;
			break;
    case VPM_WRITE:
    case TMU0_ADDR:
      printf("QPULib: Reading from write-only special register is forbidden\n");
      assert(false);
			break;
    case DUMMY:
      r.tag   = NONE;
      r.regId = v.id;
			break;
		default:
			assert(false);
			break;
  }

	return r;
}


// Translate variable to target register.
Reg dstReg(Var v) {
	using namespace QPULib::Target::instr;

  switch (v.tag) {
    case UNIFORM:
    case QPU_NUM:
    case ELEM_NUM:
    case VarTag::VPM_READ:
      fatal("QPULib: writing to read-only special register is forbidden");
			return Reg();  // Return anything

    case STANDARD:
			return Reg(REG_A, v.id);
    case VarTag::VPM_WRITE:
			return Target::instr::VPM_WRITE;
    case TMU0_ADDR:
			return TMU0_S;

		default:
			fatal("Unhandled case in dstReg()");
			return Reg();  // Return anything
  }
}


/**
 * Variable assignments
 *
 * Translate the conditional assignment of a variable to an expression.
 *
 * @param seq   Target instruction sequence to extend
 * @param cond  Condition on assignment
 * @param v     Variable on LHS
 * @param expr  Expression on RHS
 */
void varAssign(Seq<Instr>* seq, AssignCond cond, Var v, Expr* expr) {
	using namespace QPULib::Target::instr;

  Expr e = *expr;

  // -----------------------------------------
  // Case: v := w, where v and w are variables
  // -----------------------------------------
  if (e.tag == VAR) {
    Var w   = e.var;
		*seq << mov(dstReg(v), srcReg(w), cond);
    return;
  }

  // -------------------------------------------
  // Case: v := i, where i is an integer literal
  // -------------------------------------------
  if (e.tag == INT_LIT) {
		*seq << li(cond, dstReg(v), e.intLit);
    return;
  }

  // ----------------------------------------
  // Case: v := f, where f is a float literal
  // ----------------------------------------
  if (e.tag == FLOAT_LIT) {
    float f = e.floatLit;

    Instr instr(LI);
    instr.LI.cond         = cond;
    instr.LI.dest         = dstReg(v);
    instr.LI.imm.tag      = IMM_FLOAT32;
    instr.LI.imm.floatVal = f;
    seq->append(instr);
    return;
  }

  // ----------------------------------------------
  // Case: v := x op y, where x or y are not simple
  // ----------------------------------------------
  if (e.tag == APPLY &&
            (!isSimple(e.apply.lhs) ||
             !isSimple(e.apply.rhs))) {
    e.apply.lhs = simplify(seq, e.apply.lhs);
    e.apply.rhs = simplify(seq, e.apply.rhs);
  }

  // --------------------------------------------------
  // Case: v := x op y, where x and y are both literals
  // --------------------------------------------------
  if (e.tag == APPLY && isLit(e.apply.lhs) && isLit(e.apply.rhs)) {
    Var tmpVar = freshVar();
    varAssign(seq, cond, tmpVar, e.apply.lhs);
    e.apply.lhs = mkVar(tmpVar);
  }
 
  // -------------------------------------------
  // Case: v := x op y, where x and y are simple
  // -------------------------------------------
  if (e.tag == APPLY) {
    Instr instr(ALU);
    instr.ALU.cond       = cond;
    instr.ALU.dest       = dstReg(v);
    instr.ALU.srcA       = operand(e.apply.lhs);
    instr.ALU.op         = opcode(e.apply.op);
    instr.ALU.srcB       = operand(e.apply.rhs);

    *seq << instr;
    return;
  }

  // ---------------------------------------
  // Case: v := *w where w is not a variable
  // ---------------------------------------
  if (e.tag == DEREF &&
           e.deref.ptr->tag != VAR) {
    assert(!isLit(e.deref.ptr));
    e.deref.ptr = simplify(seq, e.deref.ptr);
  }

  // -----------------------------------
  // Case: v := *w where w is a variable
  // -----------------------------------
  //
  // Restriction: we disallow dereferencing in conditional ('where')
  // assignments for simplicity.  In most (all?) cases it should be
  // trivial to lift these outside the 'where'.
  //
  if (e.tag == DEREF) {
    if (!cond.is_always()) {
      printf("QPULib: dereferencing not yet supported inside 'where'\n");
      assert(false);
    }
		getSourceTranslate().varassign_deref_var(seq, v, e);
    return;
  }

  // This case should not be reachable
  assert(false);
}


void varAssign(Seq<Instr>* seq, Var v, Expr* expr) {
	varAssign(seq, always, v, expr);  // TODO: For some reason, always *must* be passed in.
	                                  //       Overloaded call generates segfault
}


/**
 * Similar to 'simplify' but ensure that the result is a variable.
 */
Expr* putInVar(Seq<Instr>* seq, Expr* e) {
	if (e->tag == VAR) {
		return e;
	}

	Var tmp = freshVar();
	varAssign(seq, tmp, e);
	return mkVar(tmp);
}


/**
 * Top-level translation function for statements.
 */
void translateStmt(Seq<Instr>* seq, Stmt* s) {
  stmt(seq, s);
	insertInitBlock(*seq);

	if (compiling_for_vc4()) {
	  insertEndCode(*seq);
	};
}


// ============================================================================
// Load/Store pass
// ============================================================================

void loadStorePass(Seq<Instr>* instrs) {
	using namespace QPULib::Target::instr;

  Seq<Instr> newInstrs(instrs->numElems*2);

  for (int i = 0; i < instrs->numElems; i++) {
    Instr instr = instrs->elems[i];
    switch (instr.tag) {
      case RECV: {
        instr.tag = TMU0_TO_ACC4;
        newInstrs << instr
				          << mov(instr.RECV.dest, ACC4);
        break;
      }
      default:
        newInstrs << instr;
        break;
    }
  }

  // Update original instruction sequence
  instrs->clear();
	*instrs << newInstrs;
}


}  // namespace QPULib
