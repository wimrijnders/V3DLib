#include "Source/Interpreter.h"
#include "Common/SharedArray.h"
#include "Source/Stmt.h"
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
  Seq<Stmt::Ptr> stack;          // Control stack
  Seq<Vec> loadBuffer;           // Load buffer
  SharedArray<uint32_t> emuHeap;

  ~CoreState() {
    // Don't delete uniform and output here, these are used as references
    delete [] env;
  }

  void store_to_heap(Vec const &index, Vec &val);
  Vec  load_from_heap(Vec const &index);
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


void CoreState::store_to_heap(Vec const &index, Vec &val) {
  assertq(index.is_uniform(), "store_to_heap(): index does not have all same values");
  assert(writeStride == 0);  // usage of writeStride is probably wrong!

  uint32_t hp = (uint32_t) index[0].intVal;  // NOTE: only first value used

  for (int i = 0; i < NUM_LANES; i++) {
    emuHeap.phy(hp>>2) = val[i].intVal;
    hp += 4 + writeStride;  // writeStride only relevant for DMA
  }
}


Vec CoreState::load_from_heap(Vec const &index) {
  assertq(index.is_uniform(), "load_from_heap(): index does not have all same values");
  assert(readStride == 0);  // Usage of readStride is probably wrong!

  uint32_t hp = (uint32_t) index[0].intVal;  // NOTE: only first value used

  Vec v;

  for (int i = 0; i < NUM_LANES; i++) {
    v[i].intVal = emuHeap.phy((hp >> 2));
    hp += 4 + readStride;  // readStride only relevant for DMA
  }

  return v;
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

}  // anon namespace


// ============================================================================
// Evaluate an arithmetic expression
// ============================================================================


Vec eval(CoreState* s, Expr::Ptr e) {
  Vec v;
  switch (e->tag()) {
    // Integer literal
    case Expr::INT_LIT:
      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = e->intLit;
      return v;

    // Float literal
    case Expr::FLOAT_LIT:
       for (int i = 0; i < NUM_LANES; i++)
        v[i].floatVal = e->floatLit;
      return v;
   
    // Variable
    case Expr::VAR:
      return evalVar(s, e->var());

    // Operator application
    case Expr::APPLY: {
      Vec a = eval(s, e->lhs());
      Vec b = eval(s, e->rhs());

      if (e->apply_op.op == ROTATE) {
        // Vector rotation
        v = rotate(a, b[0].intVal);
      } else {
        bool did_something = v.apply(e->apply_op, a, b);
        assert(did_something);
      }

      return v;
    }

    // Dereference pointer
    case Expr::DEREF:
      Vec index = eval(s, e->deref_ptr());
      v = s->load_from_heap(index);
      return v;
  }

  assert(false);
  return Vec();
}


// ============================================================================
// Evaluate boolean expression
// ============================================================================

Vec evalBool(CoreState* s, BExpr::Ptr e) {
  Vec v;

  switch (e->tag()) {
    // Negation
    case NOT:
      v = evalBool(s, e->neg());
      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = !v[i].intVal;
      return v;

    // Conjunction
    case AND: {
      Vec a = evalBool(s, e->lhs());
      Vec b = evalBool(s, e->rhs());

      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = a[i].intVal && b[i].intVal;
      return v;
    }

    // Disjunction
    case OR: {
      Vec a = evalBool(s, e->lhs());
      Vec b = evalBool(s, e->rhs());

      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = a[i].intVal || b[i].intVal;
      return v;
    }

    // Comparison
    case CMP: {
      Vec a = eval(s, e->cmp_lhs());
      Vec b = eval(s, e->cmp_rhs());
      if (e->cmp.type() == FLOAT) {
        // Floating-point comparison
        for (int i = 0; i < NUM_LANES; i++) {
          float x = a[i].floatVal;
          float y = b[i].floatVal;
          switch (e->cmp.op()) {
            case CmpOp::EQ:  v[i].intVal = x == y; break;
            case CmpOp::NEQ: v[i].intVal = x != y; break;
            case CmpOp::LT:  v[i].intVal = x <  y; break;
            case CmpOp::GT:  v[i].intVal = x >  y; break;
            case CmpOp::LE:  v[i].intVal = x <= y; break;
            case CmpOp::GE:  v[i].intVal = x >= y; break;
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

          switch (e->cmp.op()) {
            case CmpOp::EQ:  v[i].intVal = x == y; break;
            case CmpOp::NEQ: v[i].intVal = x != y; break;
            // Ideally compiler would implement:
            // case CmpOp::LT:  v[i].intVal = x <  y; break;
            // case CmpOp::GT:  v[i].intVal = x >  y; break;
            // case CmpOp::LE:  v[i].intVal = x <= y; break;
            // case CmpOp::GE:  v[i].intVal = x >= y; break;
            // But currently it implements:
            case CmpOp::LT: v[i].intVal = ((x-y) & 0x80000000) != 0; break;
            case CmpOp::GE: v[i].intVal = ((x-y) & 0x80000000) == 0; break;
            case CmpOp::LE: v[i].intVal = ((y-x) & 0x80000000) == 0; break;
            case CmpOp::GT: v[i].intVal = ((y-x) & 0x80000000) != 0; break;
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

bool evalCond(CoreState* s, CExpr::Ptr e) {
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
      break;

    case TMU0_ADDR: {  // Load via TMU
      assert(s->loadBuffer.size() < 8);
      Vec w =  s->load_from_heap(x);
      s->loadBuffer.append(w);
      break;
    }

    case VPM_WRITE:
      assertq(false, "interpreter: vpmPut() not supported");
      break;

    // Others are read-only
    case VPM_READ:
    case UNIFORM:
    case QPU_NUM:
    case ELEM_NUM:
      assertq(false, "interpreter: can not write to read-only variable");
      break;

    default:
      assertq(false, "interpreter: unexpected var-tag in assignToVar()");
      break;
  }
}


/**
 * Execute assignment
 */
void execAssign(CoreState* s, Vec cond, Expr::Ptr lhs, Expr::Ptr rhs) {
  // Evaluate RHS
  Vec val = eval(s, rhs);

  switch (lhs->tag()) {
    // Variable
    case Expr::VAR:
      assignToVar(s, cond, lhs->var(), val);
      break;

    // Dereferenced pointer
    case Expr::DEREF: {
      Vec index = eval(s, lhs->deref_ptr());
      s->store_to_heap(index, val);
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


/**
 * And two condition vectors
 */
Vec vecAnd(Vec x, Vec y) {
  Vec v;
  for (int i = 0; i < NUM_LANES; i++)
    v[i].intVal = x[i].intVal && y[i].intVal;
  return v;
}

// ============================================================================
// Execute where statement
// ============================================================================

void execWhere(CoreState* s, Vec cond, Stmt::Ptr stmt) {
  if (stmt == NULL) return;

  switch (stmt->tag) {
    // No-op
    case Stmt::GATHER_PREFETCH:
    case Stmt::SKIP:
      return;

    // Sequential composition
    case Stmt::SEQ:
      execWhere(s, cond, stmt->seq_s0());
      execWhere(s, cond, stmt->seq_s1());
      return;

    // Assignment
    case Stmt::ASSIGN:
      if (stmt->assign_lhs()->tag() != Expr::VAR) {
        printf("V3DLib: only var assignments permitted in 'where'\n");
        assert(false);
      }
      execAssign(s, cond, stmt->assign_lhs(), stmt->assign_rhs());
      return;

    // Nested where
    case Stmt::WHERE: {
      Vec b = evalBool(s, stmt->where_cond());
      execWhere(s, vecAnd(b, cond), stmt->thenStmt());
      execWhere(s, vecAnd(b.negate(), cond), stmt->elseStmt());
      return;
    }

    default:
      assertq(false, "V3DLib: only assignments and nested 'where' statements can occur in a 'where' statement");
      return;
  }
}

// ============================================================================
// Execute print statement
// ============================================================================

void execPrint(CoreState* s, Stmt::Ptr stmt) {
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

void execSetStride(CoreState* s, Stmt::Tag tag, Expr::Ptr e) {
  Vec v = eval(s, e);
  if (tag == Stmt::SET_READ_STRIDE)
    s->readStride = v[0].intVal;
  else
    s->writeStride = v[0].intVal;
}

// ============================================================================
// Execute load receive & store request statements
// ============================================================================

void execLoadReceive(CoreState* s, Expr::Ptr e) {
  assert(s->loadBuffer.size() > 0);
  assert(e->tag() == Expr::VAR);
  Vec val = s->loadBuffer.remove(0);
  assignToVar(s, Vec::Always, e->var(), val);
}


// ============================================================================
// Execute code
// ============================================================================

bool dma_exec(InterpreterState* state, CoreState* s, Stmt::Ptr &stmt) {
  bool ret = true;

  int semaId = stmt->dma.semaId();

  switch (stmt->tag) {
    // Increment semaphore
    // NOTE: emulator has a guard for protecting against loops due to semaphore waiting, perhaps also required here
    case Stmt::SEMA_INC:
      assert(semaId >= 0 && semaId < 16);
      if (state->sema[semaId] == 15) s->stack.push(stmt);
      else state->sema[semaId]++;
      break;
 
    // Decrement semaphore
    // Note at SEMA_INC also applies here
    case Stmt::SEMA_DEC:
      assert(semaId >= 0 && semaId < 16);
      if (state->sema[semaId] == 0) s->stack.push(stmt);
      else state->sema[semaId]--;
      break;

    case Stmt::DMA_READ_WAIT:
    case Stmt::DMA_WRITE_WAIT:
    case Stmt::SETUP_VPM_READ:
    case Stmt::SETUP_VPM_WRITE:
    case Stmt::SETUP_DMA_READ:
    case Stmt::SETUP_DMA_WRITE:
      // Interpreter ignores these
      break;

    case Stmt::DMA_START_READ:
    case Stmt::DMA_START_WRITE:
      fatal("V3DLib: DMA access not supported by interpreter\n");
      break;

    default:
      ret = false;
      break;
  }

  return ret;
}


void exec(InterpreterState* state, CoreState* s) {
  // Control stack must be non-empty
  assert(s->stack.size() > 0);

  // Pop the statement at the top of the stack
  Stmt::Ptr stmt = s->stack.pop();

  if (stmt == NULL) { // Apparently this happens
    //assertq(false, " Interpreter: not expecting nullptr for stmt");
    return;
  }

  if (stmt->do_break_point()) {
#ifdef DEBUG
    printf("Interpreter: hit breakpoint for stmt: %s\n", stmt->dump().c_str());
    breakpoint
#endif
  }

  switch (stmt->tag) {
    case Stmt::GATHER_PREFETCH: // Ignore
    case Stmt::SKIP:
      return;

    case Stmt::ASSIGN:          // Assignment
      execAssign(s, Vec::Always, stmt->assign_lhs(), stmt->assign_rhs());
      return;

    case Stmt::SEQ:             // Sequential composition
      s->stack.push(stmt->seq_s1());
      s->stack.push(stmt->seq_s0());
      return;

    case Stmt::WHERE: {         // Conditional assignment
      Vec b = evalBool(s, stmt->where_cond());
      execWhere(s, b, stmt->thenStmt());
      execWhere(s, b.negate(), stmt->elseStmt());
      return;
    }

    case Stmt::IF:
      if (evalCond(s, stmt->if_cond()))
        s->stack.push(stmt->thenStmt());
      else
        s->stack.push(stmt->elseStmt());
      return;

    case Stmt::WHILE:
      if (evalCond(s, stmt->loop_cond())) {
        s->stack.push(stmt);
        s->stack.push(stmt->body());
      }
      return;

    case Stmt::PRINT:
      execPrint(s, stmt);
      return;

    case Stmt::SET_READ_STRIDE:
      execSetStride(s, Stmt::SET_READ_STRIDE, stmt->dma.stride_internal());
      return;

    case Stmt::SET_WRITE_STRIDE:
      execSetStride(s, Stmt::SET_WRITE_STRIDE, stmt->dma.stride_internal());
      return;

    case Stmt::LOAD_RECEIVE:
      execLoadReceive(s, stmt->address());
      return;

    case Stmt::SEND_IRQ_TO_HOST:
      return;

    default:
      if (!dma_exec(state, s, stmt)) {
        assertq(false, "interpreter: unexpected stmt-tag in exec()");
      }
      break;
  }
}


// ============================================================================
// Interpreter
// ============================================================================

/**
 * The interpreter works in a similar way to the emulator.  The
 * difference is that the former operates on source code and the
 * latter on target code.
 *
 * @param numCores  Number of cores active
 * @param stmt      Source code
 * @param numVars   Max var id used in source
 * @param uniforms  Kernel parameters
 * @param heap
 * @param output    Output from print statements (if NULL, stdout is used)
 */
void interpreter(
  int numCores,
  Stmt::Ptr stmt,
  int numVars,
  Seq<int32_t> &uniforms,
  BufferObject &heap,
  Seq<char>* output
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
