#include "Pretty.h"
#include "Stmt.h"
#include "Syntax.h"
#include "Support/basics.h"


namespace V3DLib {

// ============================================================================
// Expressions
// ============================================================================

std::string pretty(Expr::Ptr e) {
  if (e == nullptr) {
		assert(false);
		return "";
	}
	return e->pretty();
}


// ============================================================================
// Conditional expressions
// ============================================================================

void pretty(FILE *f, CExpr* c) {
  assert(f != nullptr);
  if (c == nullptr) return;

  switch (c->tag()) {
    // Reduce using 'any'
    case ANY: fprintf(f, "any("); break;

    // Reduce using 'all'
    case ALL: fprintf(f, "all("); break;
  }

	fprintf(f, "%s", c->bexpr()->pretty().c_str());
  fprintf(f, ")");
}

// ============================================================================
// Statements
// ============================================================================

void indentBy(FILE *f, int indent) {
  for (int i = 0; i < indent; i++) fprintf(f, " ");
}


void pretty(FILE *f, int indent, Stmt::Ptr s) {
  assert(f != nullptr);
  if (s.get() == nullptr) return;

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
      fprintf(f, "%s",  pretty(s->assign_lhs()).c_str());
      fprintf(f, " = ");
      fprintf(f, "%s",  pretty(s->assign_rhs()).c_str());
      fprintf(f, ";");
			add_eol();
      break;

    // Sequential composition
    case SEQ:
      pretty(f, indent, s->seq_s0());
      pretty(f, indent, s->seq_s1());
      break;

    // Where statement
    case WHERE:
      indentBy(f, indent);
      fprintf(f, "Where (");
			fprintf(f, "%s", s->where.cond->pretty().c_str());
      fprintf(f, ")\n");
      pretty(f, indent+2, s->thenStmt());
      if (s->elseStmt().get() != nullptr) {
        indentBy(f, indent);
        fprintf(f, "Else\n");
        pretty(f, indent+2, s->elseStmt());
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
      pretty(f, indent+2, s->thenStmt());
      if (s->elseStmt().get() != nullptr) {
        indentBy(f, indent);
        fprintf(f, "Else\n");
        pretty(f, indent+2, s->elseStmt());
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
      pretty(f, indent+2, s->body());
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
      	fprintf(f, "%s", pretty(s->print_expr()).c_str());

      fprintf(f, ")\n");
      break;

    // Set read stride
    case SET_READ_STRIDE:
      indentBy(f, indent);
      fprintf(f, "dmaSetReadPitch(");
     	fprintf(f, "%s", pretty(s->stride()).c_str());
      fprintf(f, ")\n");
      break;

    // Set write stride
    case SET_WRITE_STRIDE:
      indentBy(f, indent);
      fprintf(f, "dmaSetWriteStride(");
     	fprintf(f, "%s", pretty(s->stride()).c_str());
      fprintf(f, ")\n");
      break;

    // Load receive
    case LOAD_RECEIVE:
      indentBy(f, indent);
      fprintf(f, "receive(");
     	fprintf(f, "%s", pretty(s->loadDest()).c_str());
      fprintf(f, ")");
			add_eol();
      break;

    // Store request
    case STORE_REQUEST:
      indentBy(f, indent);
      fprintf(f, "store(");
     	fprintf(f, "%s", pretty(s->storeReq_data()).c_str());
      fprintf(f, ", ");
     	fprintf(f, "%s", pretty(s->storeReq_addr()).c_str());
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
     	fprintf(f, "%s", pretty(s->setupVPMRead_addr()).c_str());
      fprintf(f, ");\n");
      break;

    // Setup VPM Write
    case SETUP_VPM_WRITE:
      indentBy(f, indent);
      fprintf(f, "vpmSetupWrite(");
      fprintf(f, "dir=%s,", s->setupVPMWrite.hor ? "HOR" : "VIR");
      fprintf(f, "stride=%i,", s->setupVPMWrite.stride);
     	fprintf(f, "%s", pretty(s->setupVPMWrite_addr()).c_str());
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
     	fprintf(f, "%s", pretty(s->startDMARead()).c_str());
      fprintf(f, ");\n");
      break;

    // DMA start write
    case DMA_START_WRITE:
      indentBy(f, indent);
      fprintf(f, "dmaStartWrite(");
     	fprintf(f, "%s", pretty(s->startDMAWrite()).c_str());
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
     	fprintf(f, "%s", pretty(s->setupDMARead_vpmAddr()).c_str());
      fprintf(f, ");\n");
      break;

    // DMA write setup
    case SETUP_DMA_WRITE:
      indentBy(f, indent);
      fprintf(f, "dmaSetupWrite(");
      fprintf(f, "numRows=%i,", s->setupDMAWrite.numRows);
      fprintf(f, "rowLen=%i,", s->setupDMAWrite.rowLen);
      fprintf(f, "dir=%s,", s->setupDMAWrite.hor ? "HORIZ" : "VERT");
     	fprintf(f, "%s", pretty(s->setupDMAWrite_vpmAddr()).c_str());
      fprintf(f, ");\n");
      break;

    // Not reachable
    default:
      assert(false);
  }
}


void pretty(FILE *f, Stmt::Ptr s) {
  assert(f != nullptr);
  pretty(f, 0, s);
}


/**
 * @brief Override using stdout as output
 */
void pretty(Stmt::Ptr s) {
  pretty(stdout, s);
}

}  // namespace V3DLib
