#include <assert.h>
#include "Source/Syntax.h"
#include "Target/Syntax.h"

namespace QPULib {

Instr genInstr(ALUOp op, AssignCond cond,  Reg dst, Reg srcA, Reg srcB) {
  Instr instr;
  instr.tag           = ALU;
  instr.ALU.setFlags  = false;
  instr.ALU.cond      = cond;
  instr.ALU.dest      = dst;
  instr.ALU.srcA.tag  = REG;
  instr.ALU.srcA.reg  = srcA;
  instr.ALU.op        = op;
  instr.ALU.srcB.tag  = REG;
  instr.ALU.srcB.reg  = srcB;

  return instr;
}


namespace {

Instr genInstr(ALUOp op, Reg dst, Reg srcA, Reg srcB) {
  AssignCond always;
  always.tag = ALWAYS;
	return genInstr(op, always, dst, srcA, srcB);
}


Instr genInstr(ALUOp op, Reg dst, Reg srcA, int n) {
  AssignCond always;
  always.tag = ALWAYS;

  Instr instr;
  instr.tag                   = ALU;
  instr.ALU.setFlags          = false;
  instr.ALU.cond              = always;
  instr.ALU.dest              = dst;
  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg          = srcA;
  instr.ALU.op                = op;
  instr.ALU.srcB.tag          = IMM;
  instr.ALU.srcB.smallImm.tag = SMALL_IMM;
  instr.ALU.srcB.smallImm.val = n;

  return instr;
}


Instr genInstr(ALUOp op, Reg dst, int n, int m) {
  AssignCond always;
  always.tag = ALWAYS;

  Instr instr;
  instr.tag                   = ALU;
  instr.ALU.setFlags          = false;
  instr.ALU.cond              = always;
  instr.ALU.dest              = dst;
  instr.ALU.srcA.tag          = IMM;
  instr.ALU.srcA.smallImm.tag = SMALL_IMM;
  instr.ALU.srcA.smallImm.val = n;
  instr.ALU.op                = op;
  instr.ALU.srcB.tag          = IMM;
  instr.ALU.srcB.smallImm.tag = SMALL_IMM;
  instr.ALU.srcB.smallImm.val = m;

  return instr;
}


/**
 * Generate addition instruction.
 */
Instr genADD(Reg dst, Reg srcA, Reg srcB) {
	return genInstr(A_ADD, dst, srcA, srcB);
}

}  // anon namespace

// =======
// Globals
// =======

// Used for fresh label generation
static int globalLabelId = 0;

// ======================
// Handy syntax functions
// ======================

// Determine if instruction is a conditional assignment
bool isCondAssign(Instr* instr)
{
  if (instr->tag == LI && instr->LI.cond.tag != ALWAYS)
    return true;
  if (instr->tag == ALU && instr->ALU.cond.tag != ALWAYS)
    return true;
  return false;
}

// Generate load-immediate instruction.

Instr genLI(AssignCond cond, Reg dst, int i) {
  AssignCond always;
  always.tag = ALWAYS;

  Instr instr;
  instr.tag           = LI;
  instr.LI.setFlags   = false;
  instr.LI.cond       = cond;
  instr.LI.dest       = dst;
  instr.LI.imm.tag    = IMM_INT32;
  instr.LI.imm.intVal = i;
 
  return instr;
}


Instr genLI(Reg dst, int i) {
  AssignCond always;
  always.tag = ALWAYS;
	return genLI(always, dst, i);
}


// Generate bitwise-or instruction.

Instr genOR(Reg dst, Reg srcA, Reg srcB) {
	return genInstr(A_BOR, dst, srcA, srcB);
}


// Generate left-shift instruction.

Instr genLShift(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
	return genInstr(A_SHL, dst, srcA, n);
}

// Generate increment instruction.

Instr genIncr(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
	return genInstr(A_ADD, dst, srcA, n);
}

// Is last instruction in a basic block?
bool isLast(Instr instr)
{
  return instr.tag == BRL || instr.tag == BR || instr.tag == END;
}

// =========================
// Fresh variable generation
// =========================

// Obtain a fresh variable
Reg freshReg()
{
  Var v = freshVar();
  Reg r;
  r.tag = REG_A;
  r.regId = v.id;
  return r;
}

// Obtain a fresh label
Label freshLabel()
{
  return globalLabelId++;
}

// Number of fresh labels
int getFreshLabelCount()
{
  return globalLabelId;
}

// Reset fresh label generator
void resetFreshLabelGen()
{
  globalLabelId = 0;
}

// Reset fresh label generator to specified value
void resetFreshLabelGen(int val)
{
  globalLabelId = val;
}


namespace Target {
namespace instr {

Reg const ACC0(ACC, 0);
Reg const ACC1(ACC, 1);
Reg const QPU_ID(SPECIAL, SPECIAL_QPU_NUM);
Reg const ELEM_ID(SPECIAL, SPECIAL_ELEM_NUM);

Reg rf(uint8_t index) {
	return Reg(REG_A, index);
}


Instr mov(Reg dst, int n) {
	return genInstr(A_BOR, dst, n, n);
}

Instr shr(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
	return genInstr(A_SHR, dst, srcA, n);
}


Instr add(Reg dst, Reg srcA, Reg srcB) {
//	breakpoint
	return genADD(dst, srcA, srcB);
}


Instr sub(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
	return genInstr(A_SUB, dst, srcA, n);
}


Instr band(Reg dst, Reg srcA, int n) {
	return genInstr(A_BAND, dst, srcA, n);
}

}  // namespace instr
}  // namespace Target

}  // namespace QPULib
