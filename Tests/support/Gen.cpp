// A random source-program generator
//
///////////////////////////////////////////////////////////////////////////////
#include "Gen.h"
#include <stdlib.h>
#include "Source/Syntax.h"
#include "Source/Cond.h"   // mkCmp()

namespace V3DLib {

namespace {

Expr::Ptr newVar() {
	return mkVar(freshVar());
};


Expr::Ptr mkFloatLit(float lit) {
	return std::make_shared<Expr>(lit);
}


// ============================================================================
// Types of expressions
// ============================================================================

// The types of the expressions we can generate are:

enum TypeTag { INT_TYPE, FLOAT_TYPE, PTR_TYPE, PTR2_TYPE };

struct Type {
  TypeTag tag;
  TypeTag ptrTo;
};

// ============================================================================
// Utilities for generating random numbers
// ============================================================================

// Return random integer in given range
int randRange(int min, int max) {
  int r = rand();
  int n = r % (1+max-min);
  return min+n;
}


// ============================================================================
// Random variables
// ============================================================================

Expr::Ptr pickVar(GenOptions* o, Type t) {
	int const QPU_ID_NUM = 2;  // Shift for qpu id and num

  int intHigh   = QPU_ID_NUM + o->numIntArgs + o->numIntVars;
  int floatLow  = intHigh   + o->depth;
  int floatHigh = floatLow  + o->numFloatArgs + o->numFloatVars;
  int ptrHigh   = floatHigh + o->numPtrArgs;
  int ptr2High  = ptrHigh   + o->numPtr2Args;

	VarId val;
  switch (t.tag) {
    case INT_TYPE:   val = randRange(QPU_ID_NUM, intHigh-1); break;
    case FLOAT_TYPE: val = randRange(floatLow, floatHigh-1); break;
    case PTR_TYPE:   val = randRange(floatHigh, ptrHigh-1); break;
    case PTR2_TYPE:  val = randRange(ptrHigh, ptr2High-1); break;
    default:         assert(false);
  }

  return mkVar(Var(STANDARD, val));
}


Expr::Ptr genVar(GenOptions* opts, Type t) {
  // Sometimes pick a QPU special variable (namely ELEM_NUM)
  if (t.tag == INT_TYPE) {
    if (randRange(0, 5) == 0) {
      return mkVar(Var(ELEM_NUM));
    }
  }

  // Otherwise, pick a standard variable
  return pickVar(opts, t);
}


Expr::Ptr genLVar(GenOptions* opts, Type t) {
  return pickVar(opts, t);
}


/**
 * Generate random float literal
 */
float genFloatLit() {
  float num   = (float) randRange(0, 1000);
  float denom = (float) randRange(1, 100);
  return num/denom;
}


// ============================================================================
// Random operators
// ============================================================================

Op genOp(GenOptions* opts, Type t) {
  switch (t.tag) {
    case INT_TYPE:
    case PTR_TYPE:
    case PTR2_TYPE: {
      OpId op = (OpId) randRange(opts->genRotate ? ROTATE : ADD, BNOT);
      return Op(op, INT32);
		}
    case FLOAT_TYPE: {
      OpId op = (OpId) randRange(opts->genRotate ? ROTATE : ADD, MAX);
      return Op(op, FLOAT);
		}
		default:
			assert(false);
			return Op(SUB, INT32);  // Return anything
  }
}


// ============================================================================
// Random arithmetic expressions
// ============================================================================

Expr::Ptr genExpr(GenOptions* opts, Type t, int depth) {
  switch (randRange(0, 3)) { 
    // Literal
    case 0: {
			// Attempt to create Literals without using stmtStack
      if (t.tag == FLOAT_TYPE) {
				return mkFloatLit(genFloatLit());
			} else if (t.tag == INT_TYPE) {
				return mkIntLit(genIntLit());
			}
    }

    // Dereference
    case 1:
      if (depth > 0 && opts->genDeref && t.tag != PTR2_TYPE) {
        Type newType;
        if (t.tag == PTR_TYPE) {
          newType.tag = PTR2_TYPE;
          newType.ptrTo = t.ptrTo;
        }
        else {
          newType.tag = PTR_TYPE;
          newType.ptrTo = t.tag;
        }
        return mkDeref(genExpr(opts, newType, depth-1));

        if (randRange(0, 1) == 0) {
          // Generate: *p where p is a pointer variable
          return mkDeref(genVar(opts, newType));
        }
        else {
          // Generate: *(p+offset) where offset is a bounded expression
          Type intType;
          intType.tag = INT_TYPE;
          Expr::Ptr offset = mkApply(genExpr(opts, intType, depth-1), Op(BAND, INT32), Int(opts->derefOffsetMask).expr());
          Expr::Ptr ptr    = mkApply(genVar(opts, newType), Op(ADD, INT32), offset);
          return mkDeref(ptr);
        }
      }

    // Application
    case 2:
      if (depth > 0) {
        // Sometimes generate a type conversion operation
        if (randRange(0, 9) == 0) {
          if (t.tag == INT_TYPE && opts->genFloat) {
            Op op = Op(FtoI, INT32);
            Type floatType;
            floatType.tag = FLOAT_TYPE;
            Expr::Ptr e = genExpr(opts, floatType, depth-1);
            return mkApply(e, op, mkIntLit(0));
          }
          else if (t.tag == FLOAT_TYPE) {
            Op op = Op(ItoF, FLOAT);
            Type intType;
            intType.tag = INT_TYPE;
            Expr::Ptr e = genExpr(opts, intType, depth-1);
            return mkApply(e, op, mkIntLit(0));
          }
        }

        // Otherwise, generate random operator application
        Expr::Ptr e1 = genExpr(opts, t, depth-1);
        Expr::Ptr e2 = genExpr(opts, t, depth-1);
        return mkApply(e1, genOp(opts, t), e2);
      }

    // Variable 
    case 3: return genVar(opts, t);
  }

  // Unreachable
  assert(false);
	return nullptr;
}


// ============================================================================
// Random boolean expressions
// ============================================================================

BExpr* genBExpr(GenOptions* opts, int depth) {
  switch (randRange(NOT, CMP)) {
    // Negation
    case NOT:
      if (depth > 0)
        return genBExpr(opts, depth-1)->Not();

    // Conjunction
    case AND:
      if (depth > 0)
        return genBExpr(opts, depth-1)->And(genBExpr(opts, depth-1));

    // Disjunction
    case OR:
      if (depth > 0)
        return genBExpr(opts, depth-1)->Or(genBExpr(opts, depth-1));

    // Comparison
    case CMP: {
      CmpOp op((CmpOpId) randRange(EQ, GE), INT32);
      if (opts->genFloat && randRange(0, 2) == 0) {
        // Floating-point comparison
        op.type = FLOAT;
        Type floatType; floatType.tag = FLOAT_TYPE;
        return mkCmp(genExpr(opts, floatType, depth-1), op, genExpr(opts, floatType, depth-1));
      } else {
        // Integer comparison
        Type intType; intType.tag = INT_TYPE;
        return mkCmp(genExpr(opts, intType, depth-1), op, genExpr(opts, intType, depth-1));
      }
    }
  }

  // Unreachable
  assert(false);
	return nullptr;
}


// ============================================================================
// Random conditional expressions
// ============================================================================

CExpr* genCExpr(GenOptions* opts, int depth) {
  switch (randRange(ALL, ANY)) {
    case ALL: return mkAll(genBExpr(opts, depth));
    case ANY: return mkAny(genBExpr(opts, depth));
  }

  // Unreachable
  assert(false);
	return nullptr;
}


// ============================================================================
// Random types
// ============================================================================

Type genType(GenOptions* opts, bool allowPointers) {
  Type t;
  if (randRange(0,1) == 0 && opts->genFloat)
    t.tag = FLOAT_TYPE;
  else
    t.tag = INT_TYPE;

  // Sometimes create a pointer type
  if (opts->genDeref && allowPointers) {
    switch (randRange(0, 5)) {
      // Pointer
      case 0:
        t.ptrTo = t.tag;
        t.tag   = PTR_TYPE;
        break;

      // Pointer to a pointer
      case 1:
        t.ptrTo = t.tag;
        t.tag   = PTR2_TYPE;
        break;
    }
  }

  return t;
}


// ============================================================================
// Random assignment statements
// ============================================================================

// Generate left-hand side of assignment statement
Expr::Ptr genLValue(GenOptions* opts, Type t, int depth) {
  // Disallow modification of pointers
  // (that would make automated testing rather tricky)
  assert(t.tag == INT_TYPE || t.tag == FLOAT_TYPE);

  if (randRange(0, 1) == 0 && opts->genDeref) {
    // Generate dereferenced expression
    t.tag   = PTR_TYPE;
    t.ptrTo = t.tag;
    return mkDeref(genExpr(opts, t, depth-1));
  }
  else {
    // Generate variable
    return genLVar(opts, t);
  }
}


/**
 * Generate assignment statement
 */
Stmt::Ptr genAssign(GenOptions* opts, int depth) {
  Type t = genType(opts, false);  // Generate random type (disallowing pointer types)
  return Stmt::create_assign(genLValue(opts, t, depth), genExpr(opts, t, depth));
}


// ============================================================================
// Random conditional assignments
// ============================================================================

Stmt::Ptr genWhere(GenOptions* opts, int depth, int length) {
  switch (randRange(0, 3)) {
    // Sequential composition
    case 0:
      if (length > 0)
        return Stmt::create_sequence(genWhere(opts, depth, 0), genWhere(opts, depth, length-1));

    // Nested where
    case 1:
      if (depth > 0)
        return mkWhere(genBExpr(opts, depth),
                       genWhere(opts, depth-1, opts->length),
                       genWhere(opts, depth-1, opts->length));
    // Skip
    case 2:
      return mkSkip();

    // Assignment
    case 3:
      return genAssign(opts, depth);
  }

  // Unreachable
  assert(false);
	return nullptr;
}


// ============================================================================
// Random while loops
// ============================================================================

Stmt::Ptr genStmt(GenOptions* opts, int depth, int length);  // Forward declaration

Stmt::Ptr genWhile(GenOptions* o, int depth) {
  assert(depth > 0 && depth <= o->depth);

  // Obtain a loop variable
  int firstLoopVar = o->numIntArgs + o->numIntVars;
  Var var(STANDARD, firstLoopVar + (depth-1));
  //Expr::Ptr v = mkVar(var);
  Expr::Ptr v = newVar();

  // Create random condition with loop bound
  BExpr* b = genBExpr(o, depth)->And(mkCmp(v, CmpOp(LT, INT32), mkIntLit(o->loopBound)));
  CExpr* c = randRange(0, 1) == 0 ? mkAny(b) : mkAll(b);

  // Initialise loop variable
  Stmt::Ptr init = Stmt::create_assign(v, mkIntLit(0));

  // Create loop increment
  Stmt::Ptr inc = Stmt::create_assign(v, mkApply(v, Op(ADD, INT32), mkIntLit(1)));

  // Create random loop body with loop increment
  Stmt::Ptr body = Stmt::create_sequence(genStmt(o, depth-1, o->length), inc);

  return Stmt::create_sequence(init, mkWhile(c, body));
}


// ============================================================================
// Random print statements
// ============================================================================

/**
 * Generate random expression and print its value
 */
Stmt::Ptr genPrint(GenOptions* opts, int depth) {
  Type t = genType(opts, false);  // Generate random type (disallowing pointer types)
  return mkPrint(t.tag == INT_TYPE ? PRINT_INT : PRINT_FLOAT, genExpr(opts, t, depth));
}


// ============================================================================
// Random statements
// ============================================================================

/**
 * Generate statement
 */
Stmt::Ptr genStmt(GenOptions* opts, int depth, int length) {
  switch (randRange(SKIP, PRINT)) {
    // Sequential composition
    case SEQ:
      if (length > 0)
        return Stmt::create_sequence(genStmt(opts, depth, 0), genStmt(opts, depth, length-1));

    // Where statement
    case WHERE:
      if (length > 0 && depth > 0)
        return mkWhere(genBExpr(opts, depth),
                       genWhere(opts, depth-1, opts->length),
                       genWhere(opts, depth-1, opts->length));

    // If statement
    case IF:
      if (length > 0 && depth > 0)
        return mkIf(genCExpr(opts, depth),
                    genStmt(opts, depth-1, opts->length),
                    genStmt(opts, depth-1, opts->length));

    // While statement
    case WHILE:
      if (length > 0 && depth > 0)
        return genWhile(opts, depth);

    // Print statement
    case PRINT:
      return genPrint(opts, depth);

    // No-op
    case SKIP:
      return mkSkip();

    // Assignment
    case ASSIGN:
      return genAssign(opts, depth);

  }

  // Not reachable
  assert(false);
	return nullptr;
}

}  // anon namespace


// ============================================================================
// Random literals
// ============================================================================

// Generate random integer literal
int genIntLit() {
  if (randRange(0,10) == 0)
    return rand();
  else
    return randRange(-50, 50);
}


// ============================================================================
// Top-level program generator
// ============================================================================

Stmt::Ptr progGen(GenOptions* opts) {
  // Initialise variables
  Stmt::Ptr pre  = mkSkip();
  Stmt::Ptr post = mkSkip();

  // Argument FIFO
  Expr::Ptr fifo = mkVar(Var(UNIFORM));

	auto assign_arg = [pre, fifo] (Stmt::Ptr &dst, Expr::Ptr v = nullptr, Expr::Ptr val = nullptr) {
		if (v.get() == nullptr) {
			v = newVar();
		}
		if (val.get() == nullptr) {
			val = fifo;
		}

		dst = Stmt::create_sequence(dst, Stmt::create_assign(v, val));
	};

	// Read qpu id and num
	assign_arg(pre);
	assign_arg(pre);

  // Read int args
  for (int i = 0; i < opts->numIntArgs; i++) {
		auto v = newVar();
    assign_arg(pre, v);
    post  = Stmt::create_sequence(post, mkPrint(PRINT_INT, v));
  }

  // Initialise int vars and loop vars
  for (int i = 0; i < opts->numIntVars + opts->depth; i++) {
		auto v = newVar();
    assign_arg(pre, v, mkIntLit(genIntLit()));
    post  = Stmt::create_sequence(post, mkPrint(PRINT_INT, v));
  }

  // Read float args
  for (int i = 0; i < opts->numFloatArgs; i++) {
		auto v = newVar();
    assign_arg(pre, v);
    post  = Stmt::create_sequence(post, mkPrint(PRINT_FLOAT, v));
  }

  // Initialise float vars
  for (int i = 0; i < opts->numFloatVars; i++) {
		auto v = newVar();
    assign_arg(pre, v, Float(genFloatLit()).expr());
    post  = Stmt::create_sequence(post, mkPrint(PRINT_FLOAT, v));
  }
 
  // Read pointer args
  for (int i = 0; i < opts->numPtrArgs + opts->numPtr2Args; i++) {
    assign_arg(pre);
  }

  // Generate statement
  Stmt::Ptr s = genStmt(opts, opts->depth, opts->length);

  return Stmt::create_sequence(pre, Stmt::create_sequence(s, post));
}

}  // namespace V3DLib
