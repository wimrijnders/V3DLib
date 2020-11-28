#include "Pretty.h"
#include "Stmt.h"
#include "Syntax.h"
#include "Support/debug.h"


namespace V3DLib {

// ============================================================================
// Operators
// ============================================================================

const char* cmpOpToString(CmpOp op) {
  switch (op.op) {
    case EQ : return "==";
    case NEQ: return "!=";
    case LT : return "<";
    case LE : return "<=";
    case GT : return ">";
    case GE : return ">=";
  }

  // Not reachable
  assert(false);
	return nullptr;
}

// ============================================================================
// Expressions
// ============================================================================

void pretty(FILE *f, Expr::Ptr e) {
  assert(f != nullptr);
  if (e == nullptr) return;

  switch (e->tag()) {
    // Integer literals
    case INT_LIT:
      fprintf(f, "%i", e->intLit);
      break;

    // Float literals
    case FLOAT_LIT:
      fprintf(f, "%f", e->floatLit);
      break;

    // Variables
    case VAR:
      if (e->var.tag() == STANDARD)
        fprintf(f, "v%i", e->var.id());
      else if (e->var.tag() == UNIFORM)
        fprintf(f, "UNIFORM");
      else if (e->var.tag() == QPU_NUM)
        fprintf(f, "QPU_NUM");
      else if (e->var.tag() == ELEM_NUM)
        fprintf(f, "ELEM_NUM");
      else if (e->var.tag() == VPM_READ)
        fprintf(f, "VPM_READ");
      else if (e->var.tag() == VPM_WRITE)
        fprintf(f, "VPM_WRITE");
      else if (e->var.tag() == TMU0_ADDR)
        fprintf(f, "TMU0_ADDR");
      break;

    // Applications
    case APPLY: {
      if (e->apply.op.noParams()) {
        fprintf(f, "%s()", e->apply.op.to_string());
      } else if (e->apply.op.isUnary()) {
        fprintf(f, "(");
        fprintf(f, "%s", e->apply.op.to_string());
        pretty(f, e->apply_lhs());
        fprintf(f, ")");
      }
      else {
        fprintf(f, "(");
        pretty(f, e->apply_lhs());
        fprintf(f, "%s", e->apply.op.to_string());
        pretty(f, e->apply_rhs());
        fprintf(f, ")");
      }
		}
		break;

    // Dereference
    case DEREF:
      fprintf(f, "*");
      pretty(f, e->deref_ptr());
      break;

  }
}

// ============================================================================
// Boolean expressions
// ============================================================================

void pretty(FILE *f, BExpr* b) {
  assert(f != nullptr);
  if (b == nullptr) return;

  switch (b->tag()) {
    // Negation
    case NOT:
      fprintf(f, "!");
      pretty(f, b->neg);
      break;

    // Conjunction
    case AND:
      fprintf(f, "(");
      pretty(f, b->conj.lhs);
      fprintf(f, " && ");
      pretty(f, b->conj.rhs);
      fprintf(f, ")");
      break;

    // Disjunction
    case OR:
      fprintf(f, "(");
      pretty(f, b->disj.lhs);
      fprintf(f, " || ");
      pretty(f, b->disj.rhs);
      fprintf(f, ")");
      break;

    // Comparison
    case CMP:
      pretty(f, b->cmp_lhs());
      fprintf(f, "%s", cmpOpToString(b->cmp.op));
      pretty(f, b->cmp_rhs());
      break;
  }
}

// ============================================================================
// Conditional expressions
// ============================================================================

void pretty(FILE *f, CExpr* c)
{
  assert(f != nullptr);
  if (c == nullptr) return;

  switch (c->tag) {
    // Reduce using 'any'
    case ANY: fprintf(f, "any("); break;

    // Reduce using 'all'
    case ALL: fprintf(f, "all("); break;
  }

  pretty(f, c->bexpr);
  fprintf(f, ")");
}

// ============================================================================
// Statements
// ============================================================================

void indentBy(FILE *f, int indent) {
  for (int i = 0; i < indent; i++) fprintf(f, " ");
}

void pretty(FILE *f, int indent, Stmt* s)
{
  assert(f != nullptr);
  if (s == nullptr) return;

	auto add_eol = [f, s] () {
		if (!s->comment().empty()) {
			fprintf(f, "                           # %s", s->comment().c_str());
		}
		fprintf(f, "\n");
	};

  switch (s->tag) {
    case SKIP: break;

    // Assignment
    case ASSIGN:
      indentBy(f, indent);
      pretty(f, s->assign.lhs);
      fprintf(f, " = ");
      pretty(f, s->assign.rhs);
      fprintf(f, ";");
			add_eol();
      break;

    // Sequential composition
    case SEQ:
      pretty(f, indent, s->seq.s0);
      pretty(f, indent, s->seq.s1);
      break;

    // Where statement
    case WHERE:
      indentBy(f, indent);
      fprintf(f, "Where (");
      pretty(f, s->where.cond);
      fprintf(f, ")\n");
      pretty(f, indent+2, s->where.thenStmt);
      if (s->where.elseStmt != nullptr) {
        indentBy(f, indent);
        fprintf(f, "Else\n");
        pretty(f, indent+2, s->where.elseStmt);
      }
      indentBy(f, indent);
      fprintf(f, "End\n");
      break;

    // If statement
    case IF:
      indentBy(f, indent);
      fprintf(f, "If (");
      pretty(f, s->ifElse.cond);
      fprintf(f, ")\n");
      pretty(f, indent+2, s->ifElse.thenStmt);
      if (s->where.elseStmt != nullptr) {
        indentBy(f, indent);
        fprintf(f, "Else\n");
        pretty(f, indent+2, s->ifElse.elseStmt);
      }
      indentBy(f, indent);
      fprintf(f, "End\n");
      break;

    // While statement
    case WHILE:
      indentBy(f, indent);
      fprintf(f, "While (");
      pretty(f, s->loop.cond);
      fprintf(f, ")\n");
      pretty(f, indent+2, s->loop.body);
      indentBy(f, indent);
      fprintf(f, "End\n");
      break;

    // Print statement
    case PRINT:
      indentBy(f, indent);
      fprintf(f, "Print (");
      if (s->print.tag() == PRINT_STR) {
        // Ideally would print escaped string here
        fprintf(f, "\"%s\"", s->print.str());
      }
      else
        pretty(f, s->print.expr());
      fprintf(f, ")\n");
      break;

    // Set read stride
    case SET_READ_STRIDE:
      indentBy(f, indent);
      fprintf(f, "dmaSetReadPitch(");
      pretty(f, s->stride);
      fprintf(f, ")\n");
      break;

    // Set write stride
    case SET_WRITE_STRIDE:
      indentBy(f, indent);
      fprintf(f, "dmaSetWriteStride(");
      pretty(f, s->stride);
      fprintf(f, ")\n");
      break;

    // Load receive
    case LOAD_RECEIVE:
      indentBy(f, indent);
      fprintf(f, "receive(");
      pretty(f, s->loadDest);
      fprintf(f, ")");
			add_eol();
      break;

    // Store request
    case STORE_REQUEST:
      indentBy(f, indent);
      fprintf(f, "store(");
      pretty(f, s->storeReq.data);
      fprintf(f, ", ");
      pretty(f, s->storeReq.addr);
      fprintf(f, ")\n");
      break;

    // Increment semaphore
    case SEMA_INC:
      indentBy(f, indent);
      fprintf(f, "semaInc(%i)\n", s->semaId);
      break;

    // Decrement semaphore
    case SEMA_DEC:
      indentBy(f, indent);
      fprintf(f, "semaDec(%i)\n", s->semaId);
      break;

    // Host IRQ
    case SEND_IRQ_TO_HOST:
      indentBy(f, indent);
      fprintf(f, "hostIRQ()\n");
      break;

    // Setup VPM Read
    case SETUP_VPM_READ:
      indentBy(f, indent);
      fprintf(f, "vpmSetupRead(");
      fprintf(f, "numVecs=%i, ", s->setupVPMRead.numVecs);
      fprintf(f, "dir=%s,", s->setupVPMRead.hor ? "HOR" : "VIR");
      fprintf(f, "stride=%i,", s->setupVPMRead.stride);
      pretty(f, s->setupVPMRead.addr);
      fprintf(f, ");\n");
      break;

    // Setup VPM Write
    case SETUP_VPM_WRITE:
      indentBy(f, indent);
      fprintf(f, "vpmSetupWrite(");
      fprintf(f, "dir=%s,", s->setupVPMWrite.hor ? "HOR" : "VIR");
      fprintf(f, "stride=%i,", s->setupVPMWrite.stride);
      pretty(f, s->setupVPMWrite.addr);
      fprintf(f, ");\n");
      break;

    // DMA read wait
    case DMA_READ_WAIT:
      indentBy(f, indent);
      fprintf(f, "dmaReadWait();\n");
      break;

    // DMA write wait
    case DMA_WRITE_WAIT:
      indentBy(f, indent);
      fprintf(f, "dmaWriteWait();\n");
      break;

    // DMA start read
    case DMA_START_READ:
      indentBy(f, indent);
      fprintf(f, "dmaStartRead(");
      pretty(f, s->startDMARead);
      fprintf(f, ");\n");
      break;

    // DMA start write
    case DMA_START_WRITE:
      indentBy(f, indent);
      fprintf(f, "dmaStartWrite(");
      pretty(f, s->startDMAWrite);
      fprintf(f, ");\n");
      break;

    // DMA read setup
    case SETUP_DMA_READ:
      indentBy(f, indent);
      fprintf(f, "dmaSetupRead(");
      fprintf(f, "numRows=%i,", s->setupDMARead.numRows);
      fprintf(f, "rowLen=%i,", s->setupDMARead.rowLen);
      fprintf(f, "dir=%s,", s->setupDMARead.hor ? "HORIZ" : "VERT");
      fprintf(f, "vpitch=%i,", s->setupDMARead.vpitch);
      pretty(f, s->setupDMARead.vpmAddr);
      fprintf(f, ");\n");
      break;

    // DMA write setup
    case SETUP_DMA_WRITE:
      indentBy(f, indent);
      fprintf(f, "dmaSetupWrite(");
      fprintf(f, "numRows=%i,", s->setupDMAWrite.numRows);
      fprintf(f, "rowLen=%i,", s->setupDMAWrite.rowLen);
      fprintf(f, "dir=%s,", s->setupDMAWrite.hor ? "HORIZ" : "VERT");
      pretty(f, s->setupDMAWrite.vpmAddr);
      fprintf(f, ");\n");
      break;

    // Not reachable
    default:
      assert(false);
  }
}

void pretty(FILE *f, Stmt* s)
{
  assert(f != nullptr);
  pretty(f, 0, s);
}


/**
 * @brief Override using stdout as output
 */
void pretty(Stmt* s)
{
  pretty(stdout, s);
}

}  // namespace V3DLib
