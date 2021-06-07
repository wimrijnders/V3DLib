#include "Mnemonics.h"
#include "Support/basics.h"

namespace V3DLib {
namespace v3d {
namespace instr {

Register const r0("r0", V3D_QPU_WADDR_R0, V3D_QPU_MUX_R0, true);
Register const r1("r1", V3D_QPU_WADDR_R1, V3D_QPU_MUX_R1, true);
Register const r2("r2", V3D_QPU_WADDR_R2, V3D_QPU_MUX_R2, true);
Register const r3("r3", V3D_QPU_WADDR_R3, V3D_QPU_MUX_R3, true);
Register const r4("r4", V3D_QPU_WADDR_R4, V3D_QPU_MUX_R4, true);
Register const r5("r5", V3D_QPU_WADDR_R5, V3D_QPU_MUX_R5);
Register const tmua("tmua", V3D_QPU_WADDR_TMUA);
Register const tmud("tmud", V3D_QPU_WADDR_TMUD);
Register const tlb("tlb", V3D_QPU_WADDR_TLB);
Register const recip("recip", V3D_QPU_WADDR_RECIP);
Register const rsqrt("rsqrt", V3D_QPU_WADDR_RSQRT);
Register const exp("exp", V3D_QPU_WADDR_EXP);
Register const log("log", V3D_QPU_WADDR_LOG);
Register const sin("sin", V3D_QPU_WADDR_SIN);
Register const rsqrt2("rsqrt2", V3D_QPU_WADDR_RSQRT2);


// For branch
BranchDest const lri("lri", V3D_QPU_WADDR_R0);


// Some obscure 'registers' in the broadcom tests
// Prefix a/r appears to indicate absolute/relative for the bdu field,
// 2nd parameter irrelevant
Register const r_unif("r_unif", V3D_QPU_WADDR_R0);
Register const a_unif("a_unif", V3D_QPU_WADDR_R0);


//////////////////////////////////////////////////////
// Instructions
//////////////////////////////////////////////////////

Instr nop() { return Instr(); }


Instr tidx(Location const &reg) {
  Instr instr;
  instr.alu_add_set_dst(reg);

  instr.sig_magic  = true;  // TODO is this really needed? Not present in eidx
  instr.alu.add.op = V3D_QPU_A_TIDX;
  instr.alu.add.a  = V3D_QPU_MUX_R1;
  instr.alu.add.b  = V3D_QPU_MUX_R0;

  return instr;
}


/**
 * Returns index of current vector item on a given QPU.
 * This will be something in the range [0..15]
 */
Instr eidx(Location const &reg) {
  Instr instr;
  instr.alu_add_set_dst(reg);

  instr.alu.add.op    = V3D_QPU_A_EIDX;
  instr.alu.add.a     = V3D_QPU_MUX_R2;
  instr.alu.add.b     = V3D_QPU_MUX_R0;

  return instr;
}


Instr itof(Location const &dst, Location const &a, SmallImm const &b) {
  Instr instr;
  instr.alu_add_set(dst, a, b);        // TODO: why would a small imm be required here??
  instr.alu.add.op = V3D_QPU_A_ITOF;
  return instr;
}


Instr ftoi(Location const &dst, Location const &a, SmallImm const &b) {
  Instr instr;
  instr.alu_add_set(dst, a, b);        // TODO: why would a small imm be required here??
  instr.alu.add.op = V3D_QPU_A_FTOIN;  // Also possible here: V3D_QPU_A_FTOIZ
                                       // TODO: Examine how to handle this, which is best
  return instr;
}


Instr shr(Location const &dst, Location const &a, SmallImm const &b) {
  Instr instr;
  instr.alu_add_set(dst, a, b);

  instr.alu.add.op    = V3D_QPU_A_SHR;
  instr.sig_magic     = true;    // TODO: need this? Also for shl?

  return instr;
}



Instr mov(Location const &dst, SmallImm const &a) { return Instr(V3D_QPU_A_OR, dst, a, a); }
Instr mov(Location const &dst, Location const &a) { return Instr(V3D_QPU_A_OR, dst, a, a); }

Instr  shl(Location const &dst, Location const &a, SmallImm const &b) { return Instr(V3D_QPU_A_SHL,  dst, a, b); }
Instr  shl(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_SHL,  dst, a, b); }
Instr  shl(Location const &dst, SmallImm const &a, SmallImm const &b) { return Instr(V3D_QPU_A_SHL,  dst, a, b); }
Instr  shl(Location const &dst, SmallImm const &a, Location const &b) { return Instr(V3D_QPU_A_SHL,  dst, a, b); }
Instr  asr(Location const &dst, Location const &a, SmallImm const &b) { return Instr(V3D_QPU_A_ASR,  dst, a, b); }
Instr  asr(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_ASR,  dst, a, b); }
Instr  add(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_ADD,  dst, a, b); }
Instr  add(Location const &dst, Location const &a, SmallImm const &b) { return Instr(V3D_QPU_A_ADD,  dst, a, b); }
Instr  add(Location const &dst, SmallImm const &a, Location const &b) { return Instr(V3D_QPU_A_ADD,  dst, a, b); }
Instr  sub(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_SUB,  dst, a, b); }
Instr  sub(Location const &dst, Location const &a, SmallImm const &b) { return Instr(V3D_QPU_A_SUB,  dst, a, b); }
Instr  sub(Location const &dst, SmallImm const &a, Location const &b) { return Instr(V3D_QPU_A_SUB,  dst, a, b); }
Instr fsub(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_FSUB, dst, a, b); }
Instr fsub(Location const &dst, SmallImm const &a, Location const &b) { return Instr(V3D_QPU_A_FSUB, dst, a, b); }
Instr fsub(Location const &dst, Location const &a, SmallImm const &b) { return Instr(V3D_QPU_A_FSUB, dst, a, b); }
Instr fadd(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_FADD, dst, a, b); }
Instr fadd(Location const &dst, Location const &a, SmallImm const &b) { return Instr(V3D_QPU_A_FADD, dst, a, b); }
Instr fadd(Location const &dst, SmallImm const &a, Location const &b) { return Instr(V3D_QPU_A_FADD, dst, a, b); }



/**
 * Same as faddf() with mux a and b reversed.
 * The op values are different to distinguish them; in the actual instruction,
 * the operation is actually the same.
 *
 * fmin/fmax have the same relation.
 */
Instr faddnf(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_FADDNF, dst, a, b); }
Instr faddnf(Location const &dst, SmallImm const &a, Location const &b) { return Instr(V3D_QPU_A_FADDNF, dst, a, b); }



///////////////////////////////////////////////////////////////////////////////
// Bitwise Operations
//
// These have prefix 'b' because the expected names are c++ keywords.
//
///////////////////////////////////////////////////////////////////////////////

Instr bor( Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_OR , dst, a, b); }
Instr bor( Location const &dst, Location const &a, SmallImm const &b) { return Instr(V3D_QPU_A_OR , dst, a, b); }
Instr bor( Location const &dst, SmallImm const &a, SmallImm const &b) { return Instr(V3D_QPU_A_OR , dst, a, b); }
Instr band(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_AND, dst, a, b); }
Instr band(Location const &dst, Location const &a, SmallImm const &b) { return Instr(V3D_QPU_A_AND, dst, a, b); }
Instr bxor(Location const &dst, Location const &a, SmallImm const &b) { return Instr(V3D_QPU_A_XOR, dst, a, b); }
Instr bxor(Location const &dst, SmallImm const &a, SmallImm const &b) { return Instr(V3D_QPU_A_XOR, dst, a, b); }

Instr fmax(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_FMAX, dst, a, b); }
Instr fcmp(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_FCMP, dst, a, b); }
Instr vfpack(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_VFPACK, dst, a, b); }
Instr vfmin(Location const &dst, SmallImm const &a, Location const &b) { return Instr(V3D_QPU_A_VFMIN, dst, a, b); }
Instr vfmin(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_VFMIN, dst, a, b); }
Instr min(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_MIN, dst, a, b); }
Instr max(Location const &dst, Location const &a, Location const &b) { return Instr(V3D_QPU_A_MAX, dst, a, b); }


Instr barrierid(v3d_qpu_waddr waddr) {
  Instr instr;

  instr.alu.add.op    = V3D_QPU_A_BARRIERID;
  instr.alu.add.a     = V3D_QPU_MUX_R4;
  instr.alu.add.b     = V3D_QPU_MUX_R2;
  instr.alu.add.waddr = waddr;

  return instr;
}


Instr vpmsetup(Register const &reg2) {
  Instr instr;

  instr.alu.add.op    = V3D_QPU_A_VPMSETUP;
  instr.alu.add.a     = reg2.to_mux();
  instr.alu.add.b     = V3D_QPU_MUX_R3;

  return instr;
}


Instr ffloor(Location const &dst, Location const &srca) {
  Instr instr;
  instr.alu_add_set_dst(dst);
  instr.alu_add_set_reg_a(srca);
  instr.alu_add_set_reg_b(r1);  // apparently implicit

  instr.alu.add.op = V3D_QPU_A_FFLOOR;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
  instr.alu.add.b_unpack = (v3d_qpu_input_unpack) V3D_QPU_A_FFLOOR; // ?? Looks wrong but matches the mesa disasm
#pragma GCC diagnostic pop


  return instr;
}


Instr flpop(RFAddress rf_addr1, RFAddress rf_addr2) {
  Instr instr;

  instr.raddr_a       = rf_addr2.to_waddr();
  instr.alu.add.op    = V3D_QPU_A_FLPOP;
  instr.alu.add.a     = V3D_QPU_MUX_A;
  instr.alu.add.b     = V3D_QPU_MUX_R4;
  instr.alu.add.waddr = rf_addr1.to_waddr();
  instr.alu.add.magic_write = false;

  return instr;
}


Instr fdx(Location const &dst, Location const &srca) {
  Instr instr;
  instr.alu_add_set_dst(dst);
  instr.alu_add_set_reg_a(srca);

  instr.alu.add.op = V3D_QPU_A_FDX;
  return instr;
}


Instr vflb(Location const &dst) {
  Instr instr;
  instr.alu_add_set_dst(dst);

  instr.alu.add.op = V3D_QPU_A_VFLB;
  return instr;
}


Instr tmuwt() {
  Instr instr;
  instr.alu.add.op = V3D_QPU_A_TMUWT;
  return instr;
}


Instr ldvpmg_in(Location const &dst, Location const &a, Location const &b) {
  return Instr(V3D_QPU_A_LDVPMG_IN, dst, a, b);
}


Instr stvpmv(SmallImm const &a, Location const &b) {
  return Instr(V3D_QPU_A_STVPMV, r0, a, b); // r0 is dummy, to align with mesa disasm
}


Instr sampid(Location const &dst) {
  return Instr(V3D_QPU_A_SAMPID, dst, r3, r3);  // Apparently r3s are implicit
}


///////////////////////////////////////////////////////////////////////////////
// Rotate instructions
///////////////////////////////////////////////////////////////////////////////

/**
 * Perform full rotate with offset in r5.
 *
 * Only mul ALU can do rotate, so this method just redirects.
 *
 * - dest is r1
 * - reg a is r0
 * - reg b if used is r5, otherwise smallimm with specific range passed (see override below)
 * - uses mov as opcode
 *
 * Since dest, src are fixed, these are not passed in.
 * If this conflicts with syntax of any other assemblers, change this
 * (it already conflicts with python6 assembler).
 *
 * ============================================================================
 * NOTES
 * -----
 * 
 * * Message from the `py-videocore6` maintainer:
 *
 *    > Yes, rotate only works on mul ALU as in VC4 QPU.
 *
 * * Rotate signal is not outputted in broadcom menmonic dump.
 *   This is most likely because rotate is of no use to MESA.
 *
 * * From python6 project(test_signals.py):
 *
 *   - nop required before rotate (but lines 82, 147 only done once before loop)
 *   - Smallimm offset in range -15,16 inclusive; 'i == offset' in points below
 *
 *   1. rot signal with rN source performs as a full rotate
 *     - nop().add(r1, r0, r0, sig = rot(i))  # Also with r5=offset, rot signal still used!
 *   2. rotate alias
 *     - rotate(r1, r0, i)       # add alias, 'i % 1 == 0' ??? Always true
 *     - nop().rotate(r1, r0, i) # mul alias
 *     - rotate(r1, r0, r5)       # add alias
 *     - nop().rotate(r1, r0, r5) # mul alias
 *   3. rot signal with rfN source performs as a quad rotate
 *     - nop().add(r1, rf32, rf32, sig = rot(i))
 *     - nop().add(r1, rf32, rf32, sig = rot(r5))
 *   4. quad_rotate alias
 *     - quad_rotate(r1, rf32, i)       # add alias
 *     - nop().quad_rotate(r1, rf32, i) # mul alias
 *     - quad_rotate(r1, rf32, r5)       # add alias
 *     - nop().quad_rotate(r1, rf32, r5) # mul alias
 *   5. instruction with r5rep dst performs as a full broadcast
 *     - Uses rot signal with special condition
 *     -  Skip for now
 *   6. broadcast alias
 *     - idem 5, skip for now
 *   7. instruction with r5 dst performs as a quad broadcast
 *     - idem 5, skip for now
 *
 * * Conclusions previous point:
 *
 *   Only 2. relevant for V3DLib code, skip rest for now
 *
 *   - nop required before rotate (but lines 82, 147 only done once before loop)
 *   - Only mul alu can do rotate (vc4 AND v3d)
 *   - dst apparently always r1
 *   - src apparently always r0 for 'full rotate'; TODO likely not true, check
 *   - offset is either a SmallImm or in r5
 *   - Smallimm offset in range -15,16 inclusive
 *   - TODO: try to understand the newfangled quad rotate shit.
 *
 */
Instr rotate(Location const &dst, Location const &a, Location const &b) {
  Instr instr;
  return instr.rotate(dst, a, b);
}


/**
 * Rotate.
 *
 * Rotate only works via the mul ALU.
 *
 * See notes in header comment of rotate overload above.
 */
Instr rotate(Location const &dst, Location const &a, SmallImm const &b) {
  Instr instr;
  return instr.rotate(dst, a, b);
}


///////////////////////////////////////////////////////////////////////////////
// Branch Instructions
///////////////////////////////////////////////////////////////////////////////

/**
 * Jump relative
 *
 * This creates an unconditionial jump.
 * Add conditions with the associated methods, eg. `na0()`
 *
 * TODO: can we get rid of this in favor of  the override?
 */
Instr branch(int target, int current) {
  Instr instr;
  instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

  instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
  instr.branch.ub = false;

  instr.branch.bdi = V3D_QPU_BRANCH_DEST_REL;  // branch dest
  instr.branch.bdu = V3D_QPU_BRANCH_DEST_REL;  // not used when branch.ub == false, just set a value

  instr.branch.msfign = V3D_QPU_MSFIGN_NONE;
  instr.branch.raddr_a = 0;

  // branch needs 4 delay slots before executing, hence the 4
  // This means that 3 more instructions will execute after the loop before jumping
  instr.branch.offset = (unsigned) 8*(target - (current + 4));

  return instr;
}


/**
 * Jump absolute
 *
 * This creates an unconditionial jump.
 * Add conditions with the associated methods, eg. `na0()`
 */
Instr branch(int target, bool relative) {
  Instr instr;
  instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

  instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
  instr.branch.ub = false;

  // branch needs 4 delay slots before executing
  // This means that 3 more instructions will execute after the loop before jumping
  // The offset value must be compensated for this

  if (relative) {
    instr.branch.bdi = V3D_QPU_BRANCH_DEST_REL;
    instr.branch.bdu = V3D_QPU_BRANCH_DEST_REL;  // not used when branch.ub == false, just set a value

    // Asumption: relative jump need not be compensated
    instr.branch.offset = (unsigned) 8*target;  // TODO check if ok
  } else {
    breakpoint
    instr.branch.bdi = V3D_QPU_BRANCH_DEST_ABS;
    instr.branch.bdu = V3D_QPU_BRANCH_DEST_ABS;  // not used when branch.ub == false, just set a value
    instr.branch.offset = (unsigned) 8*(target - 4);  // TODO check if ok
  }

  instr.branch.msfign = V3D_QPU_MSFIGN_NONE;
  instr.branch.raddr_a = 0;


  return instr;
}

/**
 * Actually called just 'b' in the mnemonics
 */
Instr bb(Location const &loc1) {
  Instr instr;
  instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

  // Values instr.sig   not important
  // Values instr.flags not important

  instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
  instr.branch.ub =  false;

  instr.branch.bdi =  V3D_QPU_BRANCH_DEST_REGFILE;
  instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;
  instr.branch.raddr_a = loc1.to_waddr();
  instr.branch.offset = 0;

  return instr;
}


Instr bb(BranchDest const &loc1) {
  Instr instr;
  instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

  // Values instr.sig   not important
  // Values instr.flags not important

  instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
  instr.branch.ub =  false;

  instr.branch.bdi =  V3D_QPU_BRANCH_DEST_LINK_REG;
  instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;
  instr.branch.raddr_a = loc1.to_waddr();
  instr.branch.offset = 0;

  return instr;
}


Instr bb(uint32_t addr) {
  Instr instr;
  instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

  // Values instr.sig   not important
  // Values instr.flags not important

  instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
  instr.branch.ub =  false;

  instr.branch.bdi =  V3D_QPU_BRANCH_DEST_ABS;
  instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;

  //instr.branch.raddr_a = loc1.to_waddr();
  instr.branch.offset = addr;

  return instr;
}


Instr bu(uint32_t addr, Location const &loc2) {
  //printf("called bu(uint32_t addr, Location const &loc2)\n");

  Instr instr;
  instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

  // Values instr.sig   not important
  // Values instr.flags not important

  instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
  instr.branch.ub =  true;

  instr.branch.bdi =  V3D_QPU_BRANCH_DEST_ABS;
  //instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;
  instr.branch.bdu =  V3D_QPU_BRANCH_DEST_REGFILE;

  instr.branch.raddr_a = loc2.to_waddr();
  instr.branch.offset = addr;

  return instr;
}


/**
 * NOTE: loc2 not used?
 */
Instr bu(BranchDest const &loc1, Location const &loc2) {
  //printf("called Instr bu(BranchDest const &loc1, Location const &loc2)\n");

  Instr instr;
  instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

  // Values instr.sig   not important
  // Values instr.flags not important

  instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
  instr.branch.ub =  true;

  instr.branch.bdi =  V3D_QPU_BRANCH_DEST_LINK_REG;

  // Hackish!
  auto bd_p = dynamic_cast<Register const *> (&loc2);
  if (bd_p == nullptr) {
    // The regular path
    instr.branch.bdu =  V3D_QPU_BRANCH_DEST_REL;
  } else {
    if (bd_p->name() == "r_unif") {
      instr.branch.bdu =  V3D_QPU_BRANCH_DEST_REL;
    } else  if (bd_p->name() == "a_unif") {
        instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;
    } else {
      assert(false);  // Not expecting anything else
    }
  }
  
  instr.branch.raddr_a = loc1.to_waddr();
  instr.branch.offset = 0;

  return instr;
}


///////////////////////////////////////////////////////////////////////////////
// SFU Instructions - NOT WORKING, they return nothing
//
// Prefix 'b' used to disambiguate, naming collision with std lib functions.
///////////////////////////////////////////////////////////////////////////////

Instr brecip(Location const &dst, Location const &a) { return Instr(V3D_QPU_A_RECIP, dst, a, r5); }    // r5 implicit
Instr brsqrt(Location const &dst, Location const &a) { return Instr(V3D_QPU_A_RSQRT, dst, a, r3); }    // r3 implicit
Instr brsqrt2(Location const &dst, Location const &a) { return Instr(V3D_QPU_A_RSQRT2, dst, a, r3); }  // r3 implicit
Instr bsin(Location const &dst, Location const &a) { return Instr(V3D_QPU_A_SIN, dst, a, a); }      // 2nd a implicit 
Instr bexp(Location const &dst, Location const &a) { return Instr(V3D_QPU_A_EXP, dst, a, r4); }        // r4 implicit
Instr blog(Location const &dst, Location const &a) { return Instr(V3D_QPU_A_LOG, dst, a, r5); }        // r5 implicit


///////////////////////////////////////////////////////////////////////////////
// Aggregated Instructions
///////////////////////////////////////////////////////////////////////////////

/**
 * a is a multiple of PI; i.e. a = 0.5 corresponds with PI/2
 *
 * Returns values for -0.5 <= a <= 0.5
 * Anything above is always 1, anything below always -1.
 */
Instructions fsin(Location const &dst, Location const &a) {
  Instructions ret;

  // bsin returns nothing, use the SFU reg instead
  ret << mov(sin, a).comment("v3d sin")
      << nop()   // This to prevent r4 used in the meantime, can be specified further
      << nop()
      << mov(dst, r4);

  return ret;
}


}  // instr
}  // v3d
}  // V3DLib
