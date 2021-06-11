#include "Pretty.h"
#include "Support/basics.h"

namespace V3DLib {
 
/**
 * Pretty printer for Target instructions
 *
 * Returns a string representation of an instruction.
 */
std::string pretty_instr(Instr const &instr) {
  std::string buf;

  switch (instr.tag) {
    case LI: {
      buf << instr.LI.cond.to_string()
          << "LI " << instr.LI.dest.dump()
          << " <-" << instr.setCond().pretty() << " "
          << instr.LI.imm.pretty();
    }
    break;

    case ALU: {
      buf << instr.ALU.cond.to_string()
          << instr.ALU.dest.dump()
          << " <-" << instr.setCond().pretty() << " "
          << instr.ALU.op.pretty();

      if (instr.ALU.noOperands()) {
        buf << "()";
      } else if (instr.ALU.oneOperand()) {
        buf << "(" << instr.ALU.srcA.disp() << ")";
      } else {
        buf << "(" << instr.ALU.srcA.disp() << ", " << instr.ALU.srcB.disp() << ")";
      }
    }
    break;

    case BR:   buf << "if " << instr.BR.cond.to_string() << " goto " << instr.BR.target.to_string(); break;
    case BRL:  buf << "if " << instr.BRL.cond.to_string() << " goto L" << instr.BRL.label;           break;
    case LAB:  buf << "L" << instr.label();                                                          break;
    case RECV: buf << "RECV(" <<  instr.RECV.dest.dump() << ")";                                     break;
    case SINC: buf << "SINC " << instr.semaId;                                                       break;
    case SDEC: buf << "SDEC " << instr.semaId;                                                       break;

    case INIT_BEGIN:
    case INIT_END:
    case END:         // vc4
    case TMU0_TO_ACC4:
    case NO_OP:
    case SKIP:
    case IRQ:
    case VPM_STALL:
    case TMUWT:
      buf << V3DLib::pretty_instr_tag(instr.tag);
      break;

    default:
      assert(false);
      buf << "<<UNKNOWN: " << instr.tag << ">>";
      break;
  }

  assert(!buf.empty());
  return buf;
}


const char *pretty_instr_tag(InstrTag tag) {
  switch(tag) {
    case LI:           return "LI";
    case ALU:          return "ALU";
    case BR:           return "BR";
    case LAB:          return "LAB";
    case NO_OP:        return "NOP";
    case SKIP:         return "SKIP";
    case END:          return "END";
    case RECV:         return "RECV";
    case IRQ:          return "IRQ";
    case VPM_STALL:    return "VPM_STALL";
    case TMU0_TO_ACC4: return "TMU0_TO_ACC4";
    case INIT_BEGIN:   return "INIT_BEGIN";
    case INIT_END:     return "INIT_END";
    case TMUWT:        return "TMUWT";

    default:
      assert(false);  // Add other tags here as required
      return "<UNKNOWN>";
  }
}


/**
 * Pretty printer for Target instructions
 *
 * Returns a string representation of an instruction.
 */
std::string pretty(Instr const &instr, bool with_comments) {
  std::string buf;

  if (with_comments && !instr.header().empty()) {
    buf << "\n# " << instr.header() << "\n";
  }

  buf << pretty_instr(instr);

  if (with_comments && !instr.comment().empty()) {
    buf << "  # " << instr.comment();
  }

  buf << "\n";
  return buf;
}

}  // namespace V3DLib
