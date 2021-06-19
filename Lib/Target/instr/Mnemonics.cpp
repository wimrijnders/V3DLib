#include "Mnemonics.h"
#include "Support/basics.h"
#include "Support/Platform.h"

namespace V3DLib {
namespace {

Instr genInstr(ALUOp::Enum op, Reg dst, Reg srcA, Reg srcB) {
  Instr instr(ALU);
  instr.ALU.cond      = always;
  instr.ALU.dest      = dst;
  instr.ALU.srcA.set_reg(srcA);
  instr.ALU.op        = ALUOp(op);
  instr.ALU.srcB.set_reg(srcB);

  return instr;
}


Instr genInstr(ALUOp::Enum op, Reg dst, Reg srcA, int n) {
  Instr instr(ALU);
  instr.ALU.dest              = dst;
  instr.ALU.srcA.set_reg(srcA);
  instr.ALU.op                = ALUOp(op);
  instr.ALU.srcB.set_imm(n);

  return instr;
}


Instr genInstr(ALUOp::Enum op, Reg dst, int n, int m) {
  Instr instr(ALU);
  instr.ALU.dest              = dst;
  instr.ALU.srcA.set_imm(n);
  instr.ALU.op                = ALUOp(op);
  instr.ALU.srcB.set_imm(m);

  return instr;
}


/**
 * SFU functions always write to ACC4
 * Also 2 NOP's required; TODO see this can be optimized
 */
Instr::List sfu_function(Var dst, Var srcA, Reg const &sfu_reg, const char *label) {
  using namespace V3DLib::Target::instr;

  Instr nop;
  Instr::List ret;

  std::string cmt = "SFU function ";
  cmt << label;

  ret << mov(sfu_reg, srcA).comment(cmt)
      << nop
      << nop
      << mov(dst, ACC4);

  return ret;
}

}  // anon namespace

namespace Target {
namespace instr {

Reg const None(NONE, 0);
Reg const ACC0(ACC, 0);
Reg const ACC1(ACC, 1);
Reg const ACC2(ACC, 2);
Reg const ACC3(ACC, 3);
Reg const ACC4(ACC, 4);
Reg const QPU_ID(       SPECIAL, SPECIAL_QPU_NUM);
Reg const ELEM_ID(      SPECIAL, SPECIAL_ELEM_NUM);
Reg const TMU0_S(       SPECIAL, SPECIAL_TMU0_S);
Reg const VPM_WRITE(    SPECIAL, SPECIAL_VPM_WRITE);
Reg const VPM_READ(     SPECIAL, SPECIAL_VPM_READ);
Reg const WR_SETUP(     SPECIAL, SPECIAL_WR_SETUP);
Reg const RD_SETUP(     SPECIAL, SPECIAL_RD_SETUP);
Reg const DMA_LD_WAIT(  SPECIAL, SPECIAL_DMA_LD_WAIT);
Reg const DMA_ST_WAIT(  SPECIAL, SPECIAL_DMA_ST_WAIT);
Reg const DMA_LD_ADDR(  SPECIAL, SPECIAL_DMA_LD_ADDR);
Reg const DMA_ST_ADDR(  SPECIAL, SPECIAL_DMA_ST_ADDR);
Reg const SFU_RECIP(    SPECIAL, SPECIAL_SFU_RECIP);
Reg const SFU_RECIPSQRT(SPECIAL, SPECIAL_SFU_RECIPSQRT);
Reg const SFU_EXP(      SPECIAL, SPECIAL_SFU_EXP);
Reg const SFU_LOG(      SPECIAL, SPECIAL_SFU_LOG);

// Synonyms for v3d
Reg const TMUD(SPECIAL, SPECIAL_VPM_WRITE);
Reg const TMUA(SPECIAL, SPECIAL_DMA_ST_ADDR);

Reg rf(uint8_t index) {
  return Reg(REG_A, index);
}


Instr mov(Var dst, Var src) { return mov(dstReg(dst), srcReg(src)); }
Instr mov(Var dst, Reg src) { return mov(dstReg(dst), src); }
Instr mov(Reg dst, Var src) { return mov(dst, srcReg(src)); }
Instr mov(Reg dst, Reg src) { return bor(dst, src, src); }

Instr mov(Reg dst, int n)   {
  if (Platform::compiling_for_vc4()) {
    return li(dst, n);
  } else {
    return genInstr(ALUOp::A_BOR, dst, n, n);
  }
}

Instr mov(Var dst, int n)   { return mov(dstReg(dst), n); }


// Generation of bitwise instructions
Instr bor(Reg dst, Reg srcA, Reg srcB)  { return genInstr(ALUOp::A_BOR, dst, srcA, srcB); }
Instr band(Reg dst, Reg srcA, Reg srcB) { return genInstr(ALUOp::A_BAND, dst, srcA, srcB); }
Instr band(Var dst, Var srcA, Var srcB) { return genInstr(ALUOp::A_BAND, dstReg(dst), srcReg(srcA), srcReg(srcB)); }
Instr band(Reg dst, Reg srcA, int n)    { return genInstr(ALUOp::A_BAND, dst, srcA, n); }
Instr bxor(Var dst, Var srcA, int n)    { return genInstr(ALUOp::A_BXOR, dstReg(dst), srcReg(srcA), n); }


/**
 * Generate left-shift instruction.
 */
Instr shl(Reg dst, Reg srcA, int val) {
  assert(val >= 0 && val <= 15);
  return genInstr(ALUOp::A_SHL, dst, srcA, val);
}


Instr shr(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
  return genInstr(ALUOp::A_SHR, dst, srcA, n);
}


/**
 * Generate addition instruction.
 */
Instr add(Reg dst, Reg srcA, Reg srcB) {
  return genInstr(ALUOp::A_ADD, dst, srcA, srcB);
}


Instr add(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
  return genInstr(ALUOp::A_ADD, dst, srcA, n);
}


Instr sub(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
  return genInstr(ALUOp::A_SUB, dst, srcA, n);
}


/**
 * Generate load-immediate instruction.
 */
Instr li(Reg dst, int i) {
  Instr instr(LI);
  instr.LI.cond = always;
  instr.LI.dest = dst;
  instr.LI.imm  = Imm(i);
 
  return instr;
}


Instr li(Var v, int i) {
  return li(dstReg(v), i).comment("li(Var, int)");
}


Instr li(Var v, float f) {
  Instr instr(LI);
  instr.LI.cond = always;
  instr.LI.dest = dstReg(v);
  instr.LI.imm  = Imm(f);
 
  return instr;
}


/**
 * Create an unconditional branch.
 *
 * Conditions can still be specified with helper methods (e.g. see `allzc()`)
 */
Instr branch(Label label) {
  Instr instr;
  instr.tag          = BRL;
  instr.BRL.cond.tag = COND_ALWAYS;    // Can still be changed
  instr.BRL.label    = label;

  return instr;
}


Instr::List recip(Var dst, Var srcA)     { return sfu_function(dst, srcA, SFU_RECIP    , "recip"); }
Instr::List recipsqrt(Var dst, Var srcA) { return sfu_function(dst, srcA, SFU_RECIPSQRT, "recipsqrt"); }
Instr::List bexp(Var dst, Var srcA)      { return sfu_function(dst, srcA, SFU_EXP      , "exp"); }
Instr::List blog(Var dst, Var srcA)      { return sfu_function(dst, srcA, SFU_LOG      , "log"); }


/**
 * Create label instruction.
 *
 * This is a meta-instruction for Target source.
 */
Instr label(Label in_label) {
  Instr instr;
  instr.tag = LAB;
  instr.label(in_label);

  return instr;
}


/**
 * Create a conditional branch.
 */
Instr branch(BranchCond cond, Label label) {
  Instr instr;
  instr.tag       = BRL;
  instr.BRL.cond  = cond; 
  instr.BRL.label = label;

  return instr;
}


/**
 * Load next value from TMU
 */
Instr recv(Reg dst) {
  Instr instr(RECV);
  instr.RECV.dest = dst;
 
  return instr;
}


/**
 * v3d only
 */
Instr tmuwt() {
  return genInstr(ALUOp::A_TMUWT, None, None, None);
}

}  // namespace instr
}  // namespace Target
}  // namespace V3DLib

