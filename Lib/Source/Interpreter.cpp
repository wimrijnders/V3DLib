#include "Source/Interpreter.h"
#include <algorithm>  // reverse()
#include "Common/SharedArray.h"
#include "Source/Stmt.h"
#include "Common/BufferObject.h"
#include "Target/EmuSupport.h"
#include "Support/basics.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace {

Vec const Always(1);
Vec const index_vec({0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});

// State of a single core.
struct CoreState {
  int id;                        // Core id
  int numCores;                  // Core count
  int nextUniform = -2;          // Pointer to next uniform to read
  int readStride = 0;            // Read stride
  int writeStride = 0;           // Write stride
  Vec* env = nullptr;            // Environment mapping vars to values
  int sizeEnv;                   // Size of the environment
  Stmts stack;                   // Control stack
  Seq<Vec> loadBuffer;           // Load buffer
  Data emuHeap;

  ~CoreState() {
    // Don't delete uniform and output here, these are used as references
    delete [] env;
  }

  void store_to_heap(Vec const &index, Vec &val);
  Vec  load_from_heap(Vec const &index);

  static void reset_count() {
    load_show_count = 0;
    store_show_count = 0;
  }

private:
  static int load_show_count;
  static int store_show_count;
};


int CoreState::load_show_count = 0;
int CoreState::store_show_count = 0;


// State of the Interpreter.
struct InterpreterState {
  CoreState core[MAX_QPUS];  // State of each core
  IntList uniforms;          // Arguments to kernel
  Word vpm[VPM_SIZE];        // Shared VPM memory
  int sema[16];              // Semaphores

  InterpreterState() {
    // Initialise semaphores
    for (int i = 0; i < 16; i++) sema[i] = 0;
  }
};


void CoreState::store_to_heap(Vec const &index, Vec &val) {
  assert(writeStride == 0);  // usage of writeStride is probably wrong!

  int const show_count = 3;

  if (!index.is_uniform()) {
    // NOTE: This part will not work for vc4 DMA output!
    //       Better to get rid of it

    std::string msg;
    msg << "store_to_heap(): index does not have all same values:" << index.dump();

    if (store_show_count == (show_count - 1)) {
      msg << "\n(this message not shown any more for more occurences)";
    }
    if (store_show_count < show_count) {
      warning(msg);
    }
    store_show_count ++;
    // The human has been warned, assume that she knows what she's doing

    for (int i = 0; i < NUM_LANES; i++) {
      uint32_t hp = (uint32_t) index[i].intVal + 4*i;  // TODO examine why '4*i' is necessary
      emuHeap.phy(hp>>2) = val[i].intVal;
    }
  } else {

    uint32_t hp = (uint32_t) index[0].intVal;  // NOTE: only first value used

    for (int i = 0; i < NUM_LANES; i++) {
      emuHeap.phy(hp>>2) = val[i].intVal;
      hp += 4 + writeStride;  // writeStride only relevant for DMA
    }
  }
}


Vec CoreState::load_from_heap(Vec const &index) {
  assert(readStride == 0);  // Usage of readStride is probably wrong!
  Vec v;

  int const show_count = 3;

  if (!index.is_uniform()) {
    std::string msg;
    msg << "load_from_heap(): index does not have all same values: " << index.dump();

    if (load_show_count == (show_count - 1)) {
      msg << "\n(this message not shown any more for more occurences)";
    }
    if (load_show_count < show_count) {
      warning(msg);
    }
    load_show_count ++;
    // The human has been warned, assume that she knows what she's doing

    for (int i = 0; i < NUM_LANES; i++) {
      uint32_t hp = (uint32_t) index[i].intVal + 4*i;
      v[i].intVal = emuHeap.phy((hp >> 2));
    }
  } else {
    uint32_t hp = (uint32_t) index[0].intVal;  // NOTE: only first value used

    for (int i = 0; i < NUM_LANES; i++) {
      v[i].intVal = emuHeap.phy((hp >> 2));
      hp += 4 + readStride;  // readStride only relevant for DMA
    }
  }

  return v;
}


// ============================================================================
// Evaluate a variable
// ============================================================================

Vec evalVar(InterpreterState &is, CoreState* s, Var v) {
  Vec x;

  switch (v.tag()) {
    case STANDARD:  // Normal variable
      assert(v.id() < s->sizeEnv);
      x = s->env[v.id()];
      break;

    case UNIFORM:  // Return next uniform
      assert(s->nextUniform < is.uniforms.size());
      if (s->nextUniform == -2)
        x = s->id;
      else if (s->nextUniform == -1)
        x = s->numCores;
      else
        x = is.uniforms[s->nextUniform];

      s->nextUniform++;
      break;

    case QPU_NUM:  // Return core id, appears to never be called. TODO cleanup?
      assertq(false, "evalVar(): QPU_NUM called", true);
      x = s->id;
      break;

    case ELEM_NUM:
      x = index_vec;
      break;

    case VPM_READ:
      assertq(false, "evalVar(): VPM_READ not supported by interpreter");
      break;

    default:
      assertq(false, "evalVar(): reading from write-only variable");
      break;

  }

  return x;
}

}  // anon namespace


// ============================================================================
// Evaluate an arithmetic expression
// ============================================================================


Vec eval(InterpreterState &is, CoreState* s, Expr::Ptr e) {
  Vec v;

  switch (e->tag()) {
    case Expr::INT_LIT:   v = e->intLit; break;
    case Expr::FLOAT_LIT: v = e->floatLit; break;
    case Expr::VAR:       v = evalVar(is, s, e->var()); break;
    case Expr::APPLY:     v.apply(e->apply_op(), eval(is, s, e->lhs()), eval(is, s, e->rhs())); break;
    case Expr::DEREF:     v = s->load_from_heap(eval(is, s, e->deref_ptr())); break;

    default:
      assertq(false, "eval(): unhandled Expr tag");
      break;
  }

  return v;
}


// ============================================================================
// Evaluate boolean expression
// ============================================================================

Vec evalBool(InterpreterState &is, CoreState* s, BExpr::Ptr e) {
  Vec v;

  switch (e->tag()) {
    // Negation
    case NOT:
      v = evalBool(is, s, e->neg());
      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = !v[i].intVal;
      return v;

    // Conjunction
    case AND: {
      Vec a = evalBool(is, s, e->lhs());
      Vec b = evalBool(is, s, e->rhs());

      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = a[i].intVal && b[i].intVal;
      return v;
    }

    // Disjunction
    case OR: {
      Vec a = evalBool(is, s, e->lhs());
      Vec b = evalBool(is, s, e->rhs());

      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = a[i].intVal || b[i].intVal;
      return v;
    }

    // Comparison
    case CMP: {
      Vec a = eval(is, s, e->cmp_lhs());
      Vec b = eval(is, s, e->cmp_rhs());
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

bool evalCond(InterpreterState &is, CoreState* s, CExpr::Ptr e) {
  Vec v = evalBool(is, s, e->bexpr());

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
void execAssign(InterpreterState &is, CoreState* s, Vec cond, Expr::Ptr lhs, Expr::Ptr rhs) {
  // Evaluate RHS
  Vec val = eval(is, s, rhs);

  switch (lhs->tag()) {
    // Variable
    case Expr::VAR:
      assignToVar(s, cond, lhs->var(), val);
      break;

    // Dereferenced pointer
    case Expr::DEREF: {
      Vec index = eval(is, s, lhs->deref_ptr());
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

void execWhere(InterpreterState &is, CoreState* s, Vec cond, Stmt::Ptr stmt) {
  if (stmt == NULL) return;

  switch (stmt->tag) {
    // No-op
    case Stmt::GATHER_PREFETCH:
    case Stmt::SKIP:
      return;

    // Sequential composition
    case Stmt::SEQ:
      execWhere(is, s, cond, stmt->seq_s0());
      execWhere(is, s, cond, stmt->seq_s1());
      return;

    // Assignment
    case Stmt::ASSIGN:
      assertq(stmt->assign_lhs()->tag() == Expr::VAR, "V3DLib: only var assignments permitted in 'where'");
      execAssign(is, s, cond, stmt->assign_lhs(), stmt->assign_rhs());
      return;

    // Nested where
    case Stmt::WHERE: {
      Vec b = evalBool(is, s, stmt->where_cond());
      execWhere(is, s, vecAnd(b, cond), stmt->thenStmt());
      execWhere(is, s, vecAnd(b.negate(), cond), stmt->elseStmt());
      return;
    }

    default:
      assertq(false, "V3DLib: only assignments and nested 'where' statements can occur in a 'where' statement");
      return;
  }
}


// ============================================================================
// Execute load receive & store request statements
// ============================================================================

void execLoadReceive(CoreState* s, Expr::Ptr e) {
  assert(s->loadBuffer.size() > 0);
  assert(e->tag() == Expr::VAR);
  Vec val = s->loadBuffer.remove(0);
  assignToVar(s, Always, e->var(), val);
}


// ============================================================================
// Execute code
// ============================================================================

bool dma_exec(InterpreterState &is, CoreState* s, Stmt::Ptr &stmt) {
  bool ret = true;

  int semaId = stmt->dma.semaId();

  switch (stmt->tag) {
    // Increment semaphore
    // NOTE: emulator has a guard for protecting against loops due to semaphore waiting, perhaps also required here
    case Stmt::SEMA_INC:
      assert(semaId >= 0 && semaId < 16);
      if (is.sema[semaId] == 15) s->stack << stmt;
      else is.sema[semaId]++;
      break;
 
    // Decrement semaphore
    // Note at SEMA_INC also applies here
    case Stmt::SEMA_DEC:
      assert(semaId >= 0 && semaId < 16);
      if (is.sema[semaId] == 0) s->stack << stmt;
      else is.sema[semaId]--;
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


void exec(InterpreterState &is, int core_index) {
  CoreState *s = &is.core[core_index];
  // Control stack must be non-empty
  assert(s->stack.size() > 0);

  // Get next statement
  Stmt::Ptr stmt = s->stack.back();
  s->stack.pop_back();

  if (stmt == NULL) { // Apparently this happens
    assertq(false, " Interpreter: not expecting nullptr for stmt", true);
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
      execAssign(is, s, Always, stmt->assign_lhs(), stmt->assign_rhs());
      return;

    case Stmt::SEQ:             // Sequential composition
      s->stack << stmt->seq_s1();
      s->stack << stmt->seq_s0();
      return;

    case Stmt::WHERE: {         // Conditional assignment
      Vec b = evalBool(is, s, stmt->where_cond());
      execWhere(is, s, b, stmt->thenStmt());
      execWhere(is, s, b.negate(), stmt->elseStmt());
      return;
    }

    case Stmt::IF:
      if (evalCond(is, s, stmt->if_cond()))
        s->stack << stmt->thenStmt();
      else if (stmt->elseStmt().get() != nullptr) {
        s->stack << stmt->elseStmt();
      }
      return;

    case Stmt::WHILE:
      if (evalCond(is, s, stmt->loop_cond())) {
        s->stack << stmt;
        s->stack << stmt->body();
      }
      return;

    case Stmt::SET_READ_STRIDE: {
      Vec v = eval(is, s, stmt->dma.stride_internal());
      s->readStride = v[0].intVal;
    }
    return;

    case Stmt::SET_WRITE_STRIDE: {
      Vec v = eval(is, s, stmt->dma.stride_internal());
      s->writeStride = v[0].intVal;
    }
    return;

    case Stmt::LOAD_RECEIVE:
      execLoadReceive(s, stmt->address());
      return;

    case Stmt::SEND_IRQ_TO_HOST:
      return;

    default:
      if (!dma_exec(is, s, stmt)) {
        assertq(false, "interpreter: unexpected stmt-tag in exec()");
      }
      break;
  }
}


// ============================================================================
// Interpreter
// ============================================================================

/**
 * Run the interpreter
 *
 * The interpreter parses the CFG ('source code') directly.
 *
 * The interpreter works in a similar way to the emulator.  The
 * difference is that the interpreter operates on source code and the
 * emulator on target code.
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
  Stmts const &stmts,
  int numVars,
  IntList &uniforms,
  BufferObject &heap
) {
  InterpreterState state;

  state.uniforms = uniforms;

  // Initialise state
  for (int i = 0; i < numCores; i++) {
    CoreState &s = state.core[i];

    s.id          = i;
    s.numCores    = numCores;
    s.env         = new Vec [numVars + 1];
    s.sizeEnv     = numVars + 1;
    s.emuHeap.heap_view(heap);
  }

  // Put statement on each core's control stack
  // Note the reversal, to use it as an actual stack
  for (int i = 0; i < numCores; i++) {
    auto &stack = state.core[i].stack;
    stack = stmts;
    std::reverse(stack.begin(), stack.end());
  }

  CoreState::reset_count();

  // Run code
  bool running = true;
  while (running) {
    running = false;
    for (int i = 0; i < numCores; i++) {
      if (state.core[i].stack.size() > 0) {
        running = true;
        exec(state, i);
      }
    }
  }
}

}  // namespace V3DLib
