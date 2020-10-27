#include "Translate.h"
#include "Support/Platform.h"
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
ALUOp opcode(Op op) {
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
				assertq(!Platform::instance().compiling_for_vc4(), "opcode(): TIDX is only for v3d", true);
				return A_TIDX;
      case EIDX: 
				assertq(!Platform::instance().compiling_for_vc4(), "opcode(): EIDX is only for v3d", true);
				return A_EIDX;
      default:
				assertq(false, "Not expecting this op for int in opcode()", true);
				break;
    }
  }

	return NOP;
}


/**
 * Translate the argument of an operator (either a variable or a small imm)
 */
RegOrImm operand(Expr* e) {
  RegOrImm x;

  if (e->tag == VAR) {
    x.tag = REG;
    x.reg = srcReg(e->var);
    return x;
  }

  int enc = encodeSmallLit(*e);
  assert(enc >= 0);
  x.tag          = IMM;
  x.smallImm.tag = SMALL_IMM;
  x.smallImm.val = enc;
  return x;
}


// ============================================================================
// Variable assignments
// ============================================================================

/**
 * Translate an expression to a simple expression, generating
 * instructions along the way.
 */
Expr *simplify(Seq<Instr>* seq, Expr* e) {
  if (e->isSimple()) {
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

// Evaluating a vector boolean expression results in a condition
// pair <condVar,condFlag> where:
//
//  * condVar is a variable containing a vector of values
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


// ============================================================================
// Boolean expressions
// ============================================================================

/**
 * Pre:
 * - Hypothesis: condVar set for condA
 * - Implicit condition vector set for condB
 *
 * ## Truth Table
 * ```
 *        | B
 * ------------------------------------
 * A      | never    | always | flag
 * ------------------------------------
 * never  | never    | always | B
 * always | always   | always | always
 * flag   | A        | always |
 * ------------------------------------
 * ```
 *
 * ------
 * ## NOTES
 *
 * 1. In the case of `cond = condB`, I would be expecting `condVar` to be completely reset,
 *    i.e. first init `condVar = 0` for all vector values.
 *
 *    This would be OK if `condA == Never` and `condvar` still zeroes.
 *
 *    TODO investigate if possible
 */
AssignCond boolOr(Seq<Instr> &seq, AssignCond condA, Var condVar, AssignCond condB) {
	using namespace Target::instr;

	//breakpoint

	// Determine a value that will cause the specified flag bit to be set in the condition vector.
	auto determineCondVar = [] (AssignCond cond) -> int {
		int val = 0;

		switch (cond.flag) {
			case ZS: val =  0; break;
			case ZC: val =  1; break;
			case NS: val = -1; break;
			case NC: val =  0; break;  // TODO set to unique value (e.g. 2) -> need to research logic for correctness
			default: assert(false); val = -1; break;
		}

		return val;
	};


  if (condA.is_always() || condB.is_always()) return always;
  if (condA.is_never()  && condB.is_never() ) return never;

  if (condB.is_never()) {                     // condA == FLAG
    seq << mov(None, condVar).SetFlags();     // Set implicit condition vector to variable
    return condA;
  } else if (condA.is_never()) {              // condB == FLAG
		int val = determineCondVar(condB);

		seq << li(condVar, val).cond(condB);      // set condB values in condVar, See Note 1
    return condB;
  } else {                                    // condA == flag and condB == FLAG
		int val = determineCondVar(condA);

		seq << li(condVar, val).cond(condB)       // Adjust condVar for condB
        << mov(None, condVar).SetFlags();     // Set implicit condition vector for new value condVar
    return condA;
  }
}


/**
 * Define conjunction using De Morgan.
 */
AssignCond boolAnd(Seq<Instr> *seq, AssignCond condA, Var condVarA, AssignCond condB) {
  return boolOr(*seq, condA.negate(), condVarA, condB.negate()).negate();
}


AssignCond cmpExp(Seq<Instr> *seq, BExpr *bexpr, Var v) {
  BExpr b = *bexpr;
  assert(b.tag == CMP);

	auto cmp_swap_leftright = [] (BExpr &b) {
    Expr *tmp   = b.cmp.lhs;
    b.cmp.lhs   = b.cmp.rhs;
    b.cmp.rhs   = tmp;
	};


 	if (b.cmp.op.op == GT) {       // 'x > y', replace with y < x
		cmp_swap_leftright(b);
    b.cmp.op.op = LT;
  }

 	if (b.cmp.op.op == LE) {       // 'x <= y', replace with y >= x
		cmp_swap_leftright(b);
    b.cmp.op.op = GE;
  }

 	if (!b.cmp.lhs->isSimple()) {  // 'x op y', where x is not simple
    b.cmp.lhs = simplify(seq, b.cmp.lhs);
  }

 	if (!b.cmp.rhs->isSimple()) {  // 'x op y', where y is not simple
    b.cmp.rhs = simplify(seq, b.cmp.rhs);
  }

 	if (isLit(b.cmp.lhs) && isLit(b.cmp.rhs)) {  // 'x op y', where x and y are both literals
    Var tmpVar = freshVar();
    varAssign(seq, tmpVar, b.cmp.lhs);
    b.cmp.lhs = mkVar(tmpVar);
  }


	//
 	// At this point x and y are simple
 	// Compute condition flag
	//
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
	Op op(SUB, b.cmp.op.type);

	Instr instr(ALU);
	instr.ALU.setFlags = true;
	instr.ALU.setCond  = set_cond;
	instr.ALU.dest     = dstReg(v);
	instr.ALU.srcA     = operand(b.cmp.lhs);
	instr.ALU.op       = opcode(op);
	instr.ALU.srcB     = operand(b.cmp.rhs);

	*seq << instr;
	return cond;
}


/**
 * Handle general boolean expressions.
 * 
 * @param seq    instruction sequence to which the instructions to evaluate the
 *               expression are appended
 * @param bexpr  the boolean expression to evaluate;
 * @param v      condVar 'v' to which the evaluated expression will be written to
 *
 * @return  the condFlagi corresponding to `v`
 *
 */
AssignCond boolExp(Seq<Instr> *seq, BExpr *bexpr, Var v) {
  BExpr b = *bexpr;

	switch (b.tag) {
		case CMP:
			return cmpExp(seq, bexpr, v);
		case NOT: {            // '!b', where b is a boolean expression
    	AssignCond cond = boolExp(seq, b.neg, v);
	    return cond.negate();
		}
		case OR: {             // 'a || b', where a, b are boolean expressions
			Var w = freshVar();
			AssignCond condA = boolExp(seq, b.disj.lhs, v);
			AssignCond condB = boolExp(seq, b.disj.rhs, w);
			return boolOr(*seq, condA, v, condB);
	  }
		case AND: {            // 'a && b', where a, b are boolean expressions
    	// Use De Morgan's law
	    BExpr* demorgan = mkNot(mkOr(mkNot(b.conj.lhs), mkNot(b.conj.rhs)));
	    return boolExp(seq, demorgan, v);
		}
		default:
  		assert(false);
			return always;       // Return anything
  }
}


// ============================================================================
// Conditional expressions
// ============================================================================

BranchCond condExp(Seq<Instr>* seq, CExpr* c) {
  Var v = freshVar();
  AssignCond cond = boolExp(seq, c->bexpr, v);

  BranchCond bcond;
  if (cond.is_always()) { bcond.tag = COND_ALWAYS; return bcond; }
  if (cond.is_never())  { bcond.tag = COND_NEVER; return bcond; }

  assert(cond.tag == AssignCond::Tag::FLAG);

  bcond.flag = cond.flag;
  if (c->tag == ANY) {
    bcond.tag = COND_ANY;
    return bcond;
  } else if (c->tag == ALL) {
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

void whereStmt(Seq<Instr> *seq, Stmt *s, Var condVar, AssignCond cond, bool saveRestore) {
  if (s == NULL) return;
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
		using Target::instr::mov;

    if (cond.is_always()) {
      // This case has a cheaper implementation

      // Compile new boolean expression
      AssignCond newCond = boolExp(seq, s->where.cond, condVar);

      // Compile 'then' statement
      if (s->where.thenStmt != NULL)
        whereStmt(seq, s->where.thenStmt, condVar, newCond, s->where.elseStmt != NULL);

      // Compile 'else' statement
      if (s->where.elseStmt != NULL)
        whereStmt(seq, s->where.elseStmt, condVar, newCond.negate(), false);
    } else {
      Var savedCondVar = freshVar();
      Var newCondVar   = freshVar();

      // Save condVar
      if (saveRestore || s->where.elseStmt != NULL)
        *seq << mov(savedCondVar, condVar);

      // Compile new boolean expression
      AssignCond newCond = boolExp(seq, s->where.cond, newCondVar);

      if (s->where.thenStmt != NULL) {
        // AND new boolean expression with original condition
        AssignCond andCond = boolAnd(seq, cond, condVar, newCond);

        // Compile 'then' statement
        whereStmt(seq, s->where.thenStmt, condVar, andCond, false);
      }

      if (saveRestore || s->where.elseStmt != NULL)
        *seq << mov(condVar, savedCondVar).SetFlags();

      if (s->where.elseStmt != NULL) {
        // AND negation of new boolean expression with original condition
        AssignCond andCond = boolAnd(seq, newCond.negate(), newCondVar, cond);
  
        // Compile 'else' statement
        whereStmt(seq, s->where.elseStmt, newCondVar, andCond, false);
  
        // Restore condVar and implicit condition vector
        if (saveRestore)
          *seq << mov(condVar, savedCondVar).SetFlags();
      }
    }

    return;
  }

  assertq(false, "QPULib: only assignments and nested 'where' statements can occur in a 'where' statement");
}


// ============================================================================
// Print statements
// ============================================================================

void printStmt(Seq<Instr> &seq, PrintStmt s) {
  Instr instr;

	auto expr_to_reg = [&seq] (PrintStmt const &s) -> Reg {
		if (s.expr->tag == VAR) {
   	  return srcReg(s.expr->var);
		} else {
      Var tmpVar = freshVar();
 	    varAssign(&seq, tmpVar, s.expr);
   	  return srcReg(tmpVar);
		}
	};

  switch (s.tag) {
    case PRINT_INT:
      instr.tag = PRI;
   	  instr.PRI = expr_to_reg(s);
    	break;
    case PRINT_FLOAT:
      instr.tag = PRF;
   	  instr.PRI = expr_to_reg(s);
    	break;
    case PRINT_STR:
      instr.tag = PRS;
      instr.PRS = s.str;
    break;
		default:
			assert(false);
			break;
  }

	seq << instr;
}


Instr loadReceive(Expr* dest) {
	assert(dest->tag == VAR);

  Instr instr(RECV);
  instr.RECV.dest = dstReg(dest->var);
	return instr;
}


void stmt(Seq<Instr>* seq, Stmt* s);  // Forward declaration

/**
 * Translate if-then-else statement to target code
 */
void translateIf(Seq<Instr> &seq, Stmt &s) {
	using namespace Target::instr;

	Label endifLabel = freshLabel();
	BranchCond cond  = condExp(&seq, s.ifElse.cond);  // Compile condition
    
	if (s.ifElse.elseStmt == NULL) {
		seq << branch(cond.negate(), endifLabel);       // Branch over 'then' statement
		stmt(&seq, s.ifElse.thenStmt);                  // Compile 'then' statement
	} else {
		Label elseLabel = freshLabel();

		seq << branch(cond.negate(), elseLabel);        // Branch to 'else' statement
		stmt(&seq, s.ifElse.thenStmt);                  // Compile 'then' statement
		seq << branch(endifLabel)                       // Branch to endif
		    << label(elseLabel);                        // Label for 'else' statement
		stmt(&seq, s.ifElse.elseStmt);                  // Compile 'else' statement
	}
	
	seq << label(endifLabel);                         // Label for endif
}


void translateWhile(Seq<Instr> &seq, Stmt &s) {
	using namespace Target::instr;

	Label startLabel = freshLabel();
	Label endLabel   = freshLabel();
	BranchCond cond  = condExp(&seq, s.loop.cond);     // Compile condition
 
	seq << branch(cond.negate(), endLabel)             // Branch over loop body
	    << label(startLabel);                          // Start label

	if (s.loop.body != NULL) stmt(&seq, s.loop.body);  // Compile body
	condExp(&seq, s.loop.cond);                        // Compute condition again
		                                                 // TODO why is this necessary?

	seq << branch(cond, startLabel)                    // Branch to start
	    << label(endLabel);                            // End label
}


// ============================================================================
// Statements
// ============================================================================

void stmt(Seq<Instr>* seq, Stmt* s) {
  if (s == nullptr) return;

  switch (s->tag) {
		case SKIP:
			break;
		case ASSIGN:             // 'lhs = rhs', where lhs and rhs are expressions
      assign(seq, s->assign.lhs, s->assign.rhs);
			break;
    case SEQ:                // 's0 ; s1', where s1 and s2 are statements
      stmt(seq, s->seq.s0);
      stmt(seq, s->seq.s1);
			break;
  	case IF:                 // 'if (c) s0 s1', where c is a condition, and s0, s1 statements
			translateIf(*seq, *s);
			break;
  	case WHILE:              // 'while (c) s', where c is a condition, and s a statement
			translateWhile(*seq, *s);
			break;
  	case WHERE: {            // 'where (b) s0 s1', where c is a boolean expr, and s0, s1 statements
	    	Var condVar = freshVar();
	    	whereStmt(seq, s, condVar, always, false);
			}
			break;
  	case PRINT:              // 'print(e)', where e is an expr or a string
	    printStmt(*seq, s->print);
			break;
  	case LOAD_RECEIVE:       // 'receive(e)', where e is an expr
	    *seq << loadReceive(s->loadDest);
			break;
		default:
			// Handle platform-specific instructions
			if (!getSourceTranslate().stmt(seq, s)) {
		  	assert(false); // Not reachable
			}
			break;
	}
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
	ret << Instr(INIT_BEGIN) << Instr(INIT_END);

	code.insert(index, ret);
}

}  // anon namespace


// ============================================================================
// Interface
// ============================================================================

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
void varAssign(Seq<Instr> *seq, AssignCond cond, Var v, Expr *expr) {
	using namespace QPULib::Target::instr;
  Expr e = *expr;

	switch (e.tag) {
		case VAR:                                                     // 'v := w', where v and w are variables
			*seq << mov(v, e.var).cond(cond);
    	break;
		case INT_LIT:                                                 // 'v := i', where i is an integer literal
			*seq << li(v, e.intLit).cond(cond);
    	break;
		case FLOAT_LIT:                                               // 'v := f', where f is a float literal
    	*seq << li(v, e.floatLit).cond(cond);
    	break;
		case APPLY: {                                                 // 'v := x op y'
			if (!e.apply.lhs->isSimple() || !e.apply.rhs->isSimple()) { // x or y are not simple
				e.apply.lhs = simplify(seq, e.apply.lhs);
				e.apply.rhs = simplify(seq, e.apply.rhs);
			}

			if (isLit(e.apply.lhs) && isLit(e.apply.rhs)) {             // x and y are both literals
				Var tmpVar = freshVar();
				varAssign(seq, cond, tmpVar, e.apply.lhs);
				e.apply.lhs = mkVar(tmpVar);
			}
			                                                            // x and y are simple
			Instr instr(ALU);
			instr.ALU.cond       = cond;
			instr.ALU.dest       = dstReg(v);
			instr.ALU.srcA       = operand(e.apply.lhs);
			instr.ALU.op         = opcode(e.apply.op);
			instr.ALU.srcB       = operand(e.apply.rhs);

			*seq << instr;
		}
		break;
		case DEREF:                                                    // 'v := *w'
			if (e.deref.ptr->tag != VAR) {                               // w is not a variable
				assert(!isLit(e.deref.ptr));
				e.deref.ptr = simplify(seq, e.deref.ptr);
			}
  		                                                             // w is a variable
			//
			// Restriction: we disallow dereferencing in conditional ('where')
			// assignments for simplicity.  In most (all?) cases it should be
			// trivial to lift these outside the 'where'.
			//
			assertq(cond.is_always(), "QPULib: dereferencing not yet supported inside 'where'");
			getSourceTranslate().varassign_deref_var(seq, v, e);
			break;
		default:
			assertq(false, "This case should not be reachable");
			break;
	}
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

	if (Platform::instance().compiling_for_vc4()) {
	  *seq << Instr(END);
	};
}


// ============================================================================
// Load/Store pass
// ============================================================================

void loadStorePass(Seq<Instr> &instrs) {
	using namespace QPULib::Target::instr;

  Seq<Instr> newInstrs(instrs.size()*2);

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];

    switch (instr.tag) {
      case RECV: {
        newInstrs << Instr(TMU0_TO_ACC4)
				          << mov(instr.RECV.dest, ACC4);
        break;
      }
      default:
        newInstrs << instr;
        break;
    }
  }

  // Update original instruction sequence
  instrs.clear();
	instrs << newInstrs;
}


}  // namespace QPULib
