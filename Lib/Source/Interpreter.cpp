#include "Source/Interpreter.h"
#include "Common/SharedArray.h"
#include "Source/Stmt.h"
#include "Source/Syntax.h"
#include "Common/BufferObject.h"
#include "Target/EmuSupport.h"
#include "Support/basics.h"

namespace V3DLib {

namespace {

// State of a single core.
struct CoreState {
  int id;                        // Core id
  int numCores;                  // Core count
  Seq<int32_t> uniforms;         // Arguments to kernel
  int nextUniform = -2;          // Pointer to next uniform to read
  int readStride = 0;            // Read stride
  int writeStride = 0;           // Write stride
  Vec* env = nullptr;            // Environment mapping vars to values
  int sizeEnv;                   // Size of the environment
  Seq<char>* output = nullptr;   // Output from print statements
  Seq<Stmt*> stack;              // Control stack
  Seq<Vec> loadBuffer;           // Load buffer
	SharedArray<uint32_t> emuHeap;


	~CoreState() {
		// Don't delete uniform and output here, these are used as references
    delete [] env;
	}
};

// State of the Interpreter.
struct InterpreterState {
  CoreState core[MAX_QPUS];  // State of each core
  Word vpm[VPM_SIZE];        // Shared VPM memory
  int sema[16];              // Semaphores

	InterpreterState() {
  	// Initialise semaphores
	  for (int i = 0; i < 16; i++) sema[i] = 0;
	}
};


void storeToHeap(CoreState *s, Vec &index, Vec &val) {
  uint32_t hp = (uint32_t) index[0].intVal;
  for (int i = 0; i < NUM_LANES; i++) {
    s->emuHeap.phy(hp>>2) = val[i].intVal;
    hp += 4 + s->writeStride;
  }
}


// ============================================================================
// Evaluate a variable
// ============================================================================

Vec evalVar(CoreState* s, Var v) {
  switch (v.tag()) {
    // Normal variable
    case STANDARD:
      assert(v.id() < s->sizeEnv);
      return s->env[v.id()];

    // Return next uniform
    case UNIFORM: {
      assert(s->nextUniform < s->uniforms.size());
      Vec x;
      for (int i = 0; i < NUM_LANES; i++)
        if (s->nextUniform == -2)
          x[i].intVal = s->id;
        else if (s->nextUniform == -1)
          x[i].intVal = s->numCores;
        else
          x[i].intVal = s->uniforms[s->nextUniform];
      s->nextUniform++;
      return x;
    }

    // Return core id
    case QPU_NUM: {
      Vec x;
      for (int i = 0; i < NUM_LANES; i++)
        x[i].intVal = s->id;
      return x;
    }

    // Return vector of integers 0..15 inclusive
    case ELEM_NUM: {
      Vec x;
      for (int i = 0; i < NUM_LANES; i++)
        x[i].intVal = i;
      return x;
    }

    // VPM read
    case VPM_READ:
      printf("V3DLib: vpmGet() not supported by interpreter\n");
      break;

    default:
      printf("V3DLib: reading from write-only variable\n");
  }

  assert(false);
	return Vec();
}


// Bitwise rotate-right
inline int32_t rotRight(int32_t x, int32_t n) {
  uint32_t ux = (uint32_t) x;
  return (ux >> n) | (x << (32-n));
}

}  // anon namespace


// ============================================================================
// Evaluate an arithmetic expression
// ============================================================================


Vec eval(CoreState* s, Expr::Ptr e) {
  Vec v;
  switch (e->tag()) {
    // Integer literal
    case INT_LIT:
      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = e->intLit;
      return v;

    // Float literal
    case FLOAT_LIT:
       for (int i = 0; i < NUM_LANES; i++)
        v[i].floatVal = e->floatLit;
      return v;
   
    // Variable
    case VAR:
      return evalVar(s, e->var);

    // Operator application
    case APPLY: {
      Vec a = eval(s, e->apply_lhs());
      Vec b = eval(s, e->apply_rhs());
      if (e->apply.op.op == ROTATE) {
        // Vector rotation
        v = rotate(a, b[0].intVal);
      }
      else if (e->apply.op.type == FLOAT) {
        // Floating-point operation
        for (int i = 0; i < NUM_LANES; i++) {
          float x = a[i].floatVal;
          float y = b[i].floatVal;
          switch (e->apply.op.op) {
            case ADD:  v[i].floatVal = x+y; break;
            case SUB:  v[i].floatVal = x-y; break;
            case MUL:  v[i].floatVal = x*y; break;
            case ItoF: v[i].floatVal = (float) a[i].intVal; break;
            case FtoI: v[i].intVal   = (int) a[i].floatVal; break;
            case MIN:  v[i].floatVal = x<y?x:y; break;
            case MAX:  v[i].floatVal = x>y?x:y; break;
            default: assert(false);
          }
        }
      }
      else {
        // Integer operation
        for (int i = 0; i < NUM_LANES; i++) {
          int32_t x   = a[i].intVal;
          int32_t y   = b[i].intVal;
          uint32_t ux = (uint32_t) x;
          switch (e->apply.op.op) {
            case ADD:  v[i].intVal = x+y; break;
            case SUB:  v[i].intVal = x-y; break;
            case MUL:  v[i].intVal = (x&0xffffff)*(y&0xffffff); break;
            case SHL:  v[i].intVal = x<<y; break;
            case SHR:  v[i].intVal = x>>y; break;
            case USHR: v[i].intVal = (int32_t) (ux >> y); break;
            case ItoF: v[i].floatVal = (float) a[i].intVal; break;
            case FtoI: v[i].intVal   = (int) a[i].floatVal; break;
            case MIN:  v[i].intVal = x<y?x:y; break;
            case MAX:  v[i].intVal = x>y?x:y; break;
            case BOR:  v[i].intVal = x|y; break;
            case BAND: v[i].intVal = x&y; break;
            case BXOR: v[i].intVal = x^y; break;
            case BNOT: v[i].intVal = ~x; break;
            case ROR:  v[i].intVal = rotRight(x, y);
            default: assert(false);
          }
        }
      }
      return v;
    }

    // Dereference pointer
    case DEREF:
      Vec a = eval(s, e->deref_ptr());
      uint32_t hp = (uint32_t) a[0].intVal;

			// NOTE: `hp` is the same for all lanes.
			//       This has been tested and verified.
			//       So, all we need to do here is add the index number to the pointer.

      Vec v;
      for (int i = 0; i < NUM_LANES; i++) {
        v[i].intVal = s->emuHeap.phy((hp >> 2) + i); // WRI added '+ i'
        hp += s->readStride;
      }
      return v;
  }

  assert(false);
	return Vec();
}


// ============================================================================
// Evaluate boolean expression
// ============================================================================

Vec evalBool(CoreState* s, BExpr *e) {
  Vec v;

  switch (e->tag()) {
    // Negation
    case NOT:
      v = evalBool(s, e->neg);
      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = !v[i].intVal;
      return v;

    // Conjunction
    case AND: {
      Vec a = evalBool(s, e->conj.lhs);
      Vec b = evalBool(s, e->conj.rhs);
      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = a[i].intVal && b[i].intVal;
      return v;
    }

    // Disjunction
    case OR: {
      Vec a = evalBool(s, e->disj.lhs);
      Vec b = evalBool(s, e->disj.rhs);
      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = a[i].intVal || b[i].intVal;
      return v;
    }

    // Comparison
    case CMP: {
      Vec a = eval(s, e->cmp_lhs());
      Vec b = eval(s, e->cmp_rhs());
      if (e->cmp.op.type == FLOAT) {
        // Floating-point comparison
        for (int i = 0; i < NUM_LANES; i++) {
          float x = a[i].floatVal;
          float y = b[i].floatVal;
          switch (e->cmp.op.op) {
            case EQ:  v[i].intVal = x == y; break;
            case NEQ: v[i].intVal = x != y; break;
            case LT:  v[i].intVal = x <  y; break;
            case GT:  v[i].intVal = x >  y; break;
            case LE:  v[i].intVal = x <= y; break;
            case GE:  v[i].intVal = x >= y; break;
            default:  assert(false);
          }
        }
        return v;
      }
      else {
        // Integer comparison
        for (int i = 0; i < NUM_LANES; i++) {
          int32_t x = a[i].intVal;
          int32_t y = b[i].intVal;
          switch (e->cmp.op.op) {
            case EQ:  v[i].intVal = x == y; break;
            case NEQ: v[i].intVal = x != y; break;
            // Ideally compiler would implement:
            // case LT:  v[i].intVal = x <  y; break;
            // case GT:  v[i].intVal = x >  y; break;
            // case LE:  v[i].intVal = x <= y; break;
            // case GE:  v[i].intVal = x >= y; break;
            // But currently it implements:
            case LT: v[i].intVal = ((x-y) & 0x80000000) != 0; break;
            case GE: v[i].intVal = ((x-y) & 0x80000000) == 0; break;
            case LE: v[i].intVal = ((y-x) & 0x80000000) == 0; break;
            case GT: v[i].intVal = ((y-x) & 0x80000000) != 0; break;
            default:  assert(false);
          }
        }
        return v;
      }
    }
  }

  // Unreachable
  assert(false);
	return Vec();
}


// ============================================================================
// Evaulate condition
// ============================================================================

bool evalCond(CoreState* s, CExpr* e) {
  Vec v = evalBool(s, e->bexpr());

  switch (e->tag()) {
    case ALL: {
      bool b = true;
      for (int i = 0; i < NUM_LANES; i++)
        b = b && v[i].intVal;
      return b;
    }
      
    case ANY: {
      bool b = false;
      for (int i = 0; i < NUM_LANES; i++)
        b = b || v[i].intVal;
      return b;
    }
  }

  // Unreachable
  assert(false);
	return false;
}


/**
 * Assign to a variable
 */
void assignToVar(CoreState* s, Vec cond, Var v, Vec x) {
  switch (v.tag()) {
    // Normal variable
    case STANDARD:
      for (int i = 0; i < NUM_LANES; i++)
        if (cond[i].intVal) {
          s->env[v.id()][i] = x[i];
        }
      return;

    // Load via TMU
    case TMU0_ADDR: {
      assert(s->loadBuffer.size() < 8);
      Vec w;
      for (int i = 0; i < NUM_LANES; i++) {
        uint32_t addr = (uint32_t) x[i].intVal;
        w[i].intVal = s->emuHeap.phy(addr>>2);
      }
      s->loadBuffer.append(w);
      return;
    }

    // VPM write
    case VPM_WRITE:
      printf("V3DLib: vpmPut() not supported by interpreter\n");
      break;

    // Others are read-only
    case UNIFORM:
    case QPU_NUM:
    case ELEM_NUM:
      printf("V3DLib: writing to read-only variable\n");
  }

  assert(false);
}


/**
 * Execute assignment
 */
void execAssign(CoreState* s, Vec cond, Expr::Ptr lhs, Expr::Ptr rhs) {
  // Evaluate RHS
  Vec val = eval(s, rhs);

  switch (lhs->tag()) {
    // Variable
    case VAR:
      assignToVar(s, cond, lhs->var, val);
      break;

    // Dereferenced pointer
		// Comparable to execStoreRequest()
    case DEREF: {
      Vec index = eval(s, lhs->deref_ptr());
			storeToHeap(s, index, val);
    }
    break;

		default:
  		assert(false);
			break;
  }
}


// ============================================================================
// Condition vector auxiliaries
// ============================================================================

// Condition vector containing all trues
Vec vecAlways()
{
  Vec always;
  for (int i = 0; i < NUM_LANES; i++)
    always[i].intVal = 1;
  return always;
}

// Negate a condition vector
Vec vecNeg(Vec cond)
{
  Vec v;
  for (int i = 0; i < NUM_LANES; i++)
    v[i].intVal = !cond[i].intVal;
  return v;
}

// And two condition vectors
Vec vecAnd(Vec x, Vec y)
{
  Vec v;
  for (int i = 0; i < NUM_LANES; i++)
    v[i].intVal = x[i].intVal && y[i].intVal;
  return v;
}

// ============================================================================
// Execute where statement
// ============================================================================

void execWhere(CoreState* s, Vec cond, Stmt* stmt)
{
  if (stmt == NULL) return;

  switch (stmt->tag) {
    // No-op
    case SKIP:
      return;

    // Sequential composition
    case SEQ:
      execWhere(s, cond, stmt->seq.s0);
      execWhere(s, cond, stmt->seq.s1);
      return;

    // Assignment
    case ASSIGN:
      if (stmt->assign_lhs()->tag() != VAR) {
        printf("V3DLib: only var assignments permitted in 'where'\n");
        assert(false);
      }
      execAssign(s, cond, stmt->assign_lhs(), stmt->assign_rhs());
      return;

    // Nested where
    case WHERE: {
      Vec b = evalBool(s, stmt->where.cond);
      execWhere(s, vecAnd(b, cond), stmt->where.thenStmt);
      execWhere(s, vecAnd(vecNeg(b), cond), stmt->where.elseStmt);
      return;
    }
  }

  printf("V3DLib: only assignments and nested 'where' \
          statements can occur in a 'where' statement\n");
  assert(false);
}

// ============================================================================
// Execute print statement
// ============================================================================

void execPrint(CoreState* s, Stmt *stmt) {
  switch (stmt->print.tag()) {
    // Integer
    case PRINT_INT: {
      Vec x = eval(s, stmt->print_expr());
      printIntVec(s->output, x);
      return;
    }

    // Float
    case PRINT_FLOAT: {
      Vec x = eval(s, stmt->print_expr());
      printFloatVec(s->output, x);
      return;
    }

    // String
    case PRINT_STR:
      emitStr(s->output, stmt->print.str());
      return;
  }
}

// ============================================================================
// Execute set-stride statements
// ============================================================================

void execSetStride(CoreState* s, StmtTag tag, Expr::Ptr e) {
  Vec v = eval(s, e);
  if (tag == SET_READ_STRIDE)
    s->readStride = v[0].intVal;
  else
    s->writeStride = v[0].intVal;
}

// ============================================================================
// Execute load receive & store request statements
// ============================================================================

void execLoadReceive(CoreState* s, Expr::Ptr e) {
  assert(s->loadBuffer.size() > 0);
  assert(e->tag() == VAR);
  Vec val = s->loadBuffer.remove(0);
  assignToVar(s, vecAlways(), e->var, val);
}


void execStoreRequest(CoreState* s, Expr::Ptr data, Expr::Ptr addr) {
  Vec val = eval(s, data);
  Vec index = eval(s, addr);
	storeToHeap(s, index, val);
}


// ============================================================================
// Execute code
// ============================================================================

void exec(InterpreterState* state, CoreState* s)
{
  // Control stack must be non-empty
  assert(s->stack.size() > 0);

  // Pop the statement at the top of the stack
  Stmt* stmt = s->stack.pop();

  if (stmt == NULL) return;

  switch (stmt->tag) {
    // No-op
    case SKIP:
      return;

    // Assignment
    case ASSIGN:
      execAssign(s, vecAlways(), stmt->assign_lhs(), stmt->assign_rhs());
      return;

    // Sequential composition
    case SEQ:
      s->stack.push(stmt->seq.s1);
      s->stack.push(stmt->seq.s0);
      return;

    // Conditional assignment
    case WHERE: {
      Vec b = evalBool(s, stmt->where.cond);
      execWhere(s, b, stmt->where.thenStmt);
      execWhere(s, vecNeg(b), stmt->where.elseStmt);
      return;
    }

    // If statement
    case IF:
      if (evalCond(s, stmt->ifElse.cond))
        s->stack.push(stmt->ifElse.thenStmt);
      else
        s->stack.push(stmt->ifElse.elseStmt);
      return;

    // While statement
    case WHILE:
      if (evalCond(s, stmt->loop.cond)) {
        s->stack.push(stmt);
        s->stack.push(stmt->loop.body);
      }
      return;

    // Print statement
    case PRINT:
      execPrint(s, stmt);
      return;

    // Set read stride
    case SET_READ_STRIDE:
      execSetStride(s, SET_READ_STRIDE, stmt->stride());
      return;

    // Set write stride
    case SET_WRITE_STRIDE:
      execSetStride(s, SET_WRITE_STRIDE, stmt->stride());
      return;

    // Load receive
    case LOAD_RECEIVE:
      execLoadReceive(s, stmt->loadDest());
      return;

    // Store request
    case STORE_REQUEST:
      execStoreRequest(s, stmt->storeReq_data(), stmt->storeReq_addr());
      return;

    // Host IRQ
    case SEND_IRQ_TO_HOST:
      return;

    // Increment semaphore
		// NOTE: emulator has a guard for protecting against loops due to semaphore waiting, perhaps also required here
    case SEMA_INC:
      assert(stmt->semaId >= 0 && stmt->semaId < 16);
      if (state->sema[stmt->semaId] == 15) s->stack.push(stmt);
      else state->sema[stmt->semaId]++;
      return;
 
    // Decrement semaphore
		// Note at SEMA_INC also applies here
    case SEMA_DEC:
      assert(stmt->semaId >= 0 && stmt->semaId < 16);
      if (state->sema[stmt->semaId] == 0) s->stack.push(stmt);
      else state->sema[stmt->semaId]--;
      return;

    case DMA_READ_WAIT:
    case DMA_WRITE_WAIT:
    case SETUP_VPM_READ:
    case SETUP_VPM_WRITE:
    case SETUP_DMA_READ:
    case SETUP_DMA_WRITE:
      // Interpreter ignores these
      return;

    case DMA_START_READ:
    case DMA_START_WRITE:
      fatal("V3DLib: DMA access not supported by interpreter\n");
      break;
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Interpreter
// ============================================================================

void interpreter(
	int numCores,           // Number of cores active
	Stmt* stmt,             // Source code
	int numVars,            // Max var id used in source
	Seq<int32_t> &uniforms, // Kernel parameters
	BufferObject &heap,
	Seq<char>* output       // Output from print statements (if NULL, stdout is used)
) {
  InterpreterState state;

  // Initialise state
  for (int i = 0; i < numCores; i++) {
    CoreState &s = state.core[i];

    s.id          = i;
    s.numCores    = numCores;
    s.uniforms    = uniforms;
    s.env         = new Vec [numVars + 1];
    s.sizeEnv     = numVars + 1;
    s.output      = output;
		s.emuHeap.heap_view(heap);
  }

	// Put statement on each core's control stack
	for (int i = 0; i < numCores; i++)
		state.core[i].stack.push(stmt);

  // Run code
  bool running = true;
  while (running) {
    running = false;
    for (int i = 0; i < numCores; i++) {
      if (state.core[i].stack.size() > 0) {
        running = true;
        exec(&state, &state.core[i]);
      }
    }
  }
}

}  // namespace V3DLib
