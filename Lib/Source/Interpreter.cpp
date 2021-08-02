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

// State of a single core.
struct CoreState {
  int id;                        // Core id
  int nextUniform = -2;          // Pointer to next uniform to read
  Seq<Vec> loadBuffer;           // Load buffer

  int readStride = 0;            // Read stride
  int writeStride = 0;           // Write stride

  Stmts stack;                   // Control stack
  Data emuHeap;

  ~CoreState() {
    delete [] m_env;
  }

  void init_env(int numVars) {
    assert(numVars >= 0);
    m_env   = new Vec [numVars + 1];
    sizeEnv = numVars + 1;
  }

  Vec &env(int env_id) {
    assert(0 <= env_id && env_id < sizeEnv);
    assert(nullptr != m_env);
    return m_env[env_id];
  } 

  void store_to_heap(Vec const &index, Vec &val);
  Vec  load_from_heap(Vec const &index);

  static void reset_count() {
    load_show_count = 0;
    store_show_count = 0;
  }

private:
  Vec *m_env  = nullptr;      // Environment mapping vars to values
  int sizeEnv = -1;           // Size of the environment

  static int load_show_count;
  static int store_show_count;
};


int CoreState::load_show_count = 0;
int CoreState::store_show_count = 0;


// State of the Interpreter.
struct InterpreterState : public EmuState {
  CoreState core[MAX_QPUS];  // State of each core

  InterpreterState(int in_num_qpus, IntList const &in_uniforms) : EmuState(in_num_qpus, in_uniforms) {} 
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
      msg << "\n(this message not shown for further occurences)";
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
      msg << "\n(this message not shown for further occurences)";
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

}  // anon namespace


// ============================================================================
// Evaluate an arithmetic expression
// ============================================================================


Vec eval(InterpreterState &is, CoreState* s, Expr::Ptr e) {
  Vec v;

  switch (e->tag()) {
    case Expr::INT_LIT:   v = e->intLit;   break;
    case Expr::FLOAT_LIT: v = e->floatLit; break;
    case Expr::APPLY:     v.apply(e->apply_op(), eval(is, s, e->lhs()), eval(is, s, e->rhs())); break;
    case Expr::DEREF:     v = s->load_from_heap(eval(is, s, e->deref_ptr())); break;

    case Expr::VAR: {
      Var var = e->var();

      switch (var.tag()) {
        case STANDARD: v = s->env(var.id()); break;
        case UNIFORM:  v = is.get_uniform(s->id, s->nextUniform); break;
        case ELEM_NUM: v = EmuState::index_vec; break;

        default:
          assertq(false, "eval(): unhandled var tag");
          break;
      }
    }
    break;

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
      for (int i = 0; i < NUM_LANES; i++) {
        if (cond[i].intVal) {
          s->env(v.id())[i] = x[i];
        }
      }
      break;

    case TMU0_ADDR: {  // Load via TMU
      assert(s->loadBuffer.size() < 8);
      Vec w =  s->load_from_heap(x);
      s->loadBuffer.append(w);
      break;
    }

    default:
      assertq(false, "assignToVar(): unhandled var-tag", true);
      break;
  }
}


/**
 * Execute assignment
 */
void execAssign(InterpreterState &is, CoreState* s, Vec cond, Expr::Ptr lhs, Expr::Ptr rhs) {
  Vec val = eval(is, s, rhs);

  switch (lhs->tag()) {
    case Expr::VAR:
      assignToVar(s, cond, lhs->var(), val);
      break;

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

void execWhere(InterpreterState &is, CoreState *s, Vec cond, Stmt::Ptr stmt);


void execWhere(InterpreterState &is, CoreState *s, Vec cond, Stmt::Array const &stmts) {
  for (int i = 0; i < (int) stmts.size(); i++) {
    execWhere(is, s, cond, stmts[i]);
  }
}


void execWhere(InterpreterState &is, CoreState *s, Vec cond, Stmt::Ptr stmt) {
  if (!stmt) return;

  switch (stmt->tag) {
    // No-ops
    case Stmt::GATHER_PREFETCH:
    case Stmt::SKIP:
      return;

    // Sequential composition
    case Stmt::SEQ: {
      breakpoint
      execWhere(is, s, cond, stmt->body());
      return;
    }

    // Assignment
    case Stmt::ASSIGN:
      assertq(stmt->assign_lhs()->tag() == Expr::VAR, "V3DLib: only var assignments permitted in 'where'");
      execAssign(is, s, cond, stmt->assign_lhs(), stmt->assign_rhs());
      return;

    // Nested where
    case Stmt::WHERE: {
      Vec b = evalBool(is, s, stmt->where_cond());
      execWhere(is, s, vecAnd(b, cond), stmt->then_block());
      execWhere(is, s, vecAnd(b.negate(), cond), stmt->else_block());
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

  switch (stmt->tag) {
    case Stmt::SET_READ_STRIDE: {
      Vec v = eval(is, s, stmt->dma.stride_internal());
      s->readStride = v[0].intVal;
    }
    break;

    case Stmt::SET_WRITE_STRIDE: {
      Vec v = eval(is, s, stmt->dma.stride_internal());
      s->writeStride = v[0].intVal;
    }
    break;

    case Stmt::SEND_IRQ_TO_HOST:
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


void append_stack(CoreState &s, Stmt::Array const &stmts) {
  // statements need to be placed reversed on the stack, which pushes and pops on the back
  for (int i = (int) stmts.size() - 1; i >= 0; i--) {
    s.stack << stmts[i];
  }
}


void exec(InterpreterState &is, int core_index) {
  CoreState *s = &is.core[core_index];
  assert(s->stack.size() > 0);

  // Get next statement
  Stmt::Ptr stmt = s->stack.back();
  s->stack.pop_back();

  if (stmt == nullptr) { // Apparently this happened
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
      break;

    case Stmt::ASSIGN:          // Assignment
      execAssign(is, s, Always, stmt->assign_lhs(), stmt->assign_rhs());
      break;

    case Stmt::SEQ:             // Sequential composition
      append_stack(*s, stmt->body());
      break;

    case Stmt::WHERE: {         // Conditional assignment
      Vec b = evalBool(is, s, stmt->where_cond());
      execWhere(is, s, b, stmt->then_block());
      execWhere(is, s, b.negate(), stmt->else_block());
    }
    break;

    case Stmt::IF:
      if (evalCond(is, s, stmt->if_cond()))
        append_stack(*s, stmt->then_block());
      else if (!stmt->else_block().empty()) {  // This test shouldn't matter much
        append_stack(*s, stmt->else_block());
      }
      break;

    case Stmt::WHILE:
      if (evalCond(is, s, stmt->loop_cond())) {
        s->stack << stmt;
        append_stack(*s, stmt->body());
      }
      break;

    case Stmt::LOAD_RECEIVE:
      execLoadReceive(s, stmt->address());
      break;

    case Stmt::SEMA_INC: if (is.sema_inc(stmt->dma.semaId())) s->stack << stmt; break;
    case Stmt::SEMA_DEC: if (is.sema_dec(stmt->dma.semaId())) s->stack << stmt; break;

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
  InterpreterState state(numCores, uniforms);

  // Initialise state
  for (int i = 0; i < numCores; i++) {
    CoreState &s = state.core[i];
    s.id          = i;
    s.init_env(numVars);
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
