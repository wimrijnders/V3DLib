#include "Pretty.h"
#include "Stmt.h"
#include "Support/basics.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace {

std::string indentBy(int indent) {
  std::string ret;
  for (int i = 0; i < indent; i++) ret += " ";
  return ret;
}


std::string pretty(int indent, Stmt::Ptr s) {
  std::string ret;
  bool do_eol = true;

  switch (s->tag) {
    case SKIP: do_eol = false; break;

    case ASSIGN:
      ret << indentBy(indent)
          << s->assign_lhs()->pretty() << " = " << s->assign_rhs()->pretty() << ";";
      break;

    case SEQ:  // Sequential composition
      ret << pretty(indent, s->seq_s0())
          << pretty(indent, s->seq_s1());
      do_eol = false;
      break;

    case WHERE:
      ret << indentBy(indent)
          << "Where (" << s->where_cond()->pretty() << ")\n"
          << pretty(indent+2, s->thenStmt());

      if (s->elseStmt().get() != nullptr) {
        ret << indentBy(indent) << "Else\n"
            << pretty(indent+2, s->elseStmt());
      }

      ret << indentBy(indent) << "End";
      break;

    case IF:
      ret << indentBy(indent)
          << "If  (" << s->if_cond()->pretty() << ")\n"
          << pretty(indent+2, s->thenStmt());

      if (s->elseStmt().get() != nullptr) {
        ret << indentBy(indent) << "Else\n"
            << pretty(indent+2, s->elseStmt());
      }

      ret << indentBy(indent) << "End";
      break;

    case WHILE:
      ret << indentBy(indent)
          << "While  (" << s->loop_cond()->pretty() << ")\n"
          << pretty(indent+2, s->body())
          << indentBy(indent) << "End";
      break;

    case PRINT:
      ret << indentBy(indent)
          << "Print (";

      if (s->print.tag() == PRINT_STR) {
        ret << s->print.str();
      } else {
        ret << s->print_expr()->pretty();
      }

      ret << ")\n";
      break;

    case SET_READ_STRIDE:
      ret << indentBy(indent) << "dmaSetReadPitch(" << s->stride()->pretty() << ");";
      break;

    case SET_WRITE_STRIDE:
      ret << indentBy(indent) << "dmaSetWriteStride(" << s->stride()->pretty() << ")";
      break;

    case LOAD_RECEIVE:
      ret << indentBy(indent)
          << "receive(" << s->address()->pretty() << ")";
      break;

    case STORE_REQUEST:
      ret << indentBy(indent)
          << "store(" << s->storeReq_data()->pretty() << ", " << s->storeReq_addr()->pretty() << ")\n";
      break;

    case SEMA_INC:  // Increment semaphore
      ret << indentBy(indent) << "semaInc(" << s->semaId << ");";
      break;

    case SEMA_DEC: // Decrement semaphore
      ret << indentBy(indent) << "semaDec(" << s->semaId << ");";
      break;

    case SEND_IRQ_TO_HOST:
      ret << indentBy(indent) << "hostIRQ();";
      break;

    case SETUP_VPM_READ:
      ret << indentBy(indent)
          << "vpmSetupRead("
          << "numVecs=" << s->setupVPMRead.numVecs               << ","
          << "dir="     << (s->setupVPMRead.hor ? "HOR" : "VIR") << ","
          << "stride="  << s->setupVPMRead.stride                << ","
          << s->address()->pretty()
          << ");";
      break;

    case SETUP_VPM_WRITE:
      ret << indentBy(indent)
          << "vpmSetupWrite("
          << "dir="    << (s->setupVPMWrite.hor ? "HOR" : "VIR") << ","
          << "stride=" << s->setupVPMWrite.stride                << ","
          << s->address()->pretty()
          << ");";
      break;

    case DMA_READ_WAIT:
      ret << indentBy(indent) << "dmaReadWait();";
      break;

    case DMA_WRITE_WAIT:
      ret << indentBy(indent) << "dmaWriteWait();";
      break;

    case DMA_START_READ:
      ret << indentBy(indent)
          << "dmaStartRead(" << s->address()->pretty() << ");";
      break;

    case DMA_START_WRITE:
      ret << indentBy(indent)
          << "dmaStartWrite(" << s->address()->pretty() << ");";
      break;

    case SETUP_DMA_READ:
      ret << indentBy(indent)
          << "dmaSetupRead("
          << "numRows=" << s->setupDMARead.numRows                  << ","
          << "rowLen=%" << s->setupDMARead.rowLen                   << ","
          << "dir="     << (s->setupDMARead.hor ? "HORIZ" : "VERT") << ","
          << "vpitch="  <<  s->setupDMARead.vpitch                  << ","
          << s->address()->pretty()
          << ");";
      break;

    case SETUP_DMA_WRITE:
      ret << indentBy(indent)
          << "dmaSetupWrite("
          << "numRows=" << s->setupDMAWrite.numRows                  << ","
          << "rowLen="  << s->setupDMAWrite.rowLen                   << ","
          << "dir="     << (s->setupDMAWrite.hor ? "HORIZ" : "VERT") << ","
          << s->address()->pretty()
          << ");";
      break;

    // Not reachable
    default:
      assert(false);
      break;
  }


  if (ret.empty()) {  // Can only be empty for SKIP
    return ret;
  }

  std::string out;
  out << s->emit_header()
      << ret

      // NOTE: For multiline output, the comment gets added to the end!
      //       We might want to fix this.
      << s->emit_comment(27);  // param is temp measure till we figure out size of current instruction
                               // TODO fix this (too lazy now)


  if (do_eol) {
    out << "\n";
  }

  return out;
}

}  // anon namespace


/**
 * Pretty printer for the V3DLib source language
 */
std::string pretty(Stmt::Ptr s) {
  assert(s.get() != nullptr);
  return pretty(0, s);
}

}  // namespace V3DLib
