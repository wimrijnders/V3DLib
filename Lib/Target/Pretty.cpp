#include "Target/Syntax.h"
#include "Target/SmallLiteral.h"

namespace QPULib {

namespace {

const char* pretty(SubWord sw)
{
  switch (sw) {
    case A8:  return "[7:0]";
    case B8:  return "[15:8]";
    case C8:  return "[23:16]";
    case D8:  return "[31:24]";
    case A16: return "[15:0]";
    case B16: return "[31:16]";
    default:  assert(false); return "";
  }
}

const char* specialStr(RegId rid)
{
  Special s = (Special) rid;
  switch (s) {
    case SPECIAL_UNIFORM:      return "UNIFORM";
    case SPECIAL_ELEM_NUM:     return "ELEM_NUM";
    case SPECIAL_QPU_NUM:      return "QPU_NUM";
    case SPECIAL_RD_SETUP:     return "RD_SETUP";
    case SPECIAL_WR_SETUP:     return "WR_SETUP";
    case SPECIAL_DMA_ST_ADDR:  return "DMA_ST_ADDR";
    case SPECIAL_DMA_LD_ADDR:  return "DMA_LD_ADDR";
    case SPECIAL_DMA_ST_WAIT:  return "DMA_ST_WAIT";
    case SPECIAL_DMA_LD_WAIT:  return "DMA_LD_WAIT";
    case SPECIAL_VPM_READ:     return "VPM_READ";
    case SPECIAL_VPM_WRITE:    return "VPM_WRITE";
    case SPECIAL_HOST_INT:     return "HOST_INT";
    case SPECIAL_TMU0_S:       return "TMU0_S";
  }

  // Unreachable
  assert(false);
	return nullptr;
}

void pretty(FILE *f, Reg r)
{
  switch (r.tag) {
    case REG_A:
      fprintf(f, "A%i", r.regId);
      return;
    case REG_B: fprintf(f, "B%i", r.regId); return;
    case ACC: fprintf(f, "ACC%i", r.regId); return;
    case SPECIAL: fprintf(f, "S[%s]", specialStr(r.regId)); return;
    case NONE: fprintf(f, "_"); return;
  }
}

void pretty(FILE *f, Flag flag)
{
  switch (flag) {
    case ZS: fprintf(f, "ZS"); return;
    case ZC: fprintf(f, "ZC"); return;
    case NS: fprintf(f, "NS"); return;
    case NC: fprintf(f, "NC"); return;
  }
}

void pretty(FILE *f, BranchCond cond)
{
  switch (cond.tag) {
    case COND_ALL:
      fprintf(f, "all(");
      pretty(f, cond.flag);
      fprintf(f, ")");
      return;
    case COND_ANY:
      fprintf(f, "any(");
      pretty(f, cond.flag);
      fprintf(f, ")");
      return;
    case COND_ALWAYS:
      fprintf(f, "always");
      return;
    case COND_NEVER:
      fprintf(f, "never");
      return;
  }
}


void pretty(FILE *f, AssignCond cond) {
	using Tag = AssignCond::Tag;

  switch (cond.tag) {
    case Tag::ALWAYS: fprintf(f, "always"); return;
    case Tag::NEVER: fprintf(f, "never"); return;
    case Tag::FLAG: pretty(f, cond.flag); return;
  }
}

void pretty(FILE *f, Imm imm) {
  switch (imm.tag) {
    case IMM_INT32:
      fprintf(f, "%d", imm.intVal);
      return;
    case IMM_FLOAT32:
      fprintf(f, "%f", imm.floatVal);
      return;
    case IMM_MASK:
      int b = imm.intVal;
      for (int i = 0; i < 16; i++) {
        fprintf(f, "%i", b&1 ? 1 : 0);
        b >>= 1;
      }
      return;
  }
}

void pretty(FILE *f, SmallImm imm)
{
  switch (imm.tag) {
    case SMALL_IMM: printSmallLit(f, imm.val); return;
    case ROT_ACC: fprintf(f, "ROT(ACC5)"); return;
    case ROT_IMM: fprintf(f, "ROT(%i)", imm.val); return;
  }
}

void pretty(FILE *f, RegOrImm r)
{
  switch (r.tag) {
    case REG: pretty(f, r.reg); return;
    case IMM: pretty(f, r.smallImm); return;
  }
}


const char *pretty_op(ALUOp op) {
  switch (op) {
    case NOP:       return "nop";
    case A_FADD:    return "addf";
    case A_FSUB:    return "subf";
    case A_FMIN:    return "minf";
    case A_FMAX:    return "maxf";
    case A_FMINABS: return "minabsf";
    case A_FMAXABS: return "maxabsf";
    case A_FtoI:    return "ftoi";
    case A_ItoF:    return "itof";
    case A_ADD:     return "add";
    case A_SUB:     return "sub";
    case A_SHR:     return "shr";
    case A_ASR:     return "asr";
    case A_ROR:     return "ror";
    case A_SHL:     return "shl";
    case A_MIN:     return "min";
    case A_MAX:     return "max";
    case A_BAND:    return "and";
    case A_BOR:     return "or";
    case A_BXOR:    return "xor";
    case A_BNOT:    return "not";
    case A_CLZ:     return "clz";
    case A_V8ADDS:  return "addsatb";
    case A_V8SUBS:  return "subsatb";
    case M_FMUL:    return "mulf";
    case M_MUL24:   return "mul24";
    case M_V8MUL:   return "mulb";
    case M_V8MIN:   return "minb";
    case M_V8MAX:   return "maxb";
    case M_V8ADDS:  return "m_addsatb";
    case M_V8SUBS:  return "m_subsatb";
    case M_ROTATE:  return "rotate";
		default:
			assert(false);
			return "";
  }
}


std::string pretty_conditions(Instr const &instr) {
	std::string ret;

	auto set_cond = [] (SetCond cond) -> const char * {
		switch (cond) {
			case NO_COND: return "None";
			case Z:       return "Z";
			case N:       return "N";
			case C:       return "C";
			default:
				assert(false);
				return "";
		}
	};

  switch (instr.tag) {
    case LI:
			if (instr.LI.setFlags) {
				ret += "{sf-";
				ret += set_cond(instr.LI.setCond);
				ret += "}";
			}
			break;
    case ALU:
			if (instr.ALU.setFlags) {
				ret += "{sf-";
				ret += set_cond(instr.ALU.setCond);
				ret += "}";
			}
			break;
	}

	return ret;
}


void pretty(FILE *f, BranchTarget target)
{
  if (target.relative)
    fprintf(f, "PC+1+");
  if (target.useRegOffset)
    fprintf(f, "A%i+", target.regOffset);
  fprintf(f, "%i", target.immOffset);
}

}  // anon namespace


const char *pretty_instr_tag(InstrTag tag) {
	switch(tag) {
		case LI:           return "LI";
		case ALU:          return "ALU";
		case BR:           return "BR";
		case LAB:          return "LAB";
		case NO_OP:        return "NOP";
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


void pretty(FILE *f, Instr instr, int index) {
	auto ALWAYS = AssignCond::Tag::ALWAYS;
  assert(f != nullptr);

	if (!instr.header().empty()) {
		fprintf(f, "\n# %s\n", instr.header().c_str());
	}

	fprintf(f, "%i: ", index);

  switch (instr.tag) {
    case LI:
      if (instr.LI.cond.tag != ALWAYS) {
        fprintf(f, "where ");
        pretty(f, instr.LI.cond);
        fprintf(f, ": ");
      }
      fprintf(f, "LI ");
      pretty(f, instr.LI.dest);
      fprintf(f, " <-%s ", pretty_conditions(instr).c_str());
      pretty(f, instr.LI.imm);
      break;
    case ALU:
      if (instr.ALU.cond.tag != ALWAYS) {
        fprintf(f, "where ");
        pretty(f, instr.ALU.cond);
        fprintf(f, ": ");
      }
      pretty(f, instr.ALU.dest);
      fprintf(f, " <-%s ", pretty_conditions(instr).c_str());
      fprintf(f, "%s", pretty_op(instr.ALU.op));
      fprintf(f, "(");
      pretty(f, instr.ALU.srcA);
      fprintf(f, ", ");
      pretty(f, instr.ALU.srcB);
      fprintf(f, ")");
      break;
    case BR:
      fprintf(f, "if ");
      pretty(f, instr.BR.cond);
      fprintf(f, " goto ");
      pretty(f, instr.BR.target);
      break;
    case BRL:
      fprintf(f, "if ");
      pretty(f, instr.BRL.cond);
      fprintf(f, " goto L%i", instr.BRL.label);
      break;
    case LAB:
      fprintf(f, "L%i:", instr.label());
      break;
    case PRS:
      fprintf(f, "PRS(\"%s\")", instr.PRS);
      break;
    case PRI:
      fprintf(f, "PRI(");
      pretty(f, instr.PRI);
      fprintf(f, ")");
      break;
    case PRF:
      fprintf(f, "PRF(");
      pretty(f, instr.PRF);
      fprintf(f, ")");
      break;
    case RECV:
      fprintf(f, "RECV(");
      pretty(f, instr.RECV.dest);
      fprintf(f, ")");
      break;
    case SINC:
      fprintf(f, "SINC %i", instr.semaId);
      break;;
    case SDEC:
      fprintf(f, "SDEC %i", instr.semaId);
      break;
    case INIT_BEGIN:
    case INIT_END:
    case END:         // vc4
    case TMU0_TO_ACC4:
    case NO_OP:
    case IRQ:
    case VPM_STALL:
    case TMUWT:
      fprintf(f, "%s", pretty_instr_tag(instr.tag));
      break;
    default:
      fprintf(f, "<<UNKNOWN:%d>>", instr.tag);
      break;
  }

	if (!instr.comment().empty()) {
		fprintf(f, "  # %s\n", instr.comment().c_str());
	} else {
		fprintf(f, "\n");
	}
}

}  // namespace QPULib
