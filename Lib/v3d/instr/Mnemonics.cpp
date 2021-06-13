#include "Mnemonics.h"
#include "Support/basics.h"

namespace V3DLib {
namespace v3d {
namespace instr {

///////////////////////////////////////////////////////////////////////////////
// Class Mnemonic 
///////////////////////////////////////////////////////////////////////////////

Mnemonic::Mnemonic(v3d_qpu_add_op op, Location const &dst, Source const &a, Source const &b) {
  init(NOP);
  alu_add_set(dst, a, b);
  alu.add.op = op;
}


/**
 * Initialize the add alu
 */
Mnemonic::Mnemonic(v3d_qpu_add_op op, Location const &dst, Location const &a, Location const &b) {
  init(NOP);
  alu_add_set(dst, a, b);
  alu.add.op = op;
}


/**
 * Initialize the add alu
 */
Mnemonic::Mnemonic(v3d_qpu_add_op op, Location const &dst, Location const &a, SmallImm const &b) {
  init(NOP);
  alu_add_set(dst, a, b);
  alu.add.op = op;
}


/**
 * Initialize the add alu
 */
Mnemonic::Mnemonic(v3d_qpu_add_op op, Location const &dst, SmallImm const &a, Location const &b) {
  init(NOP);
  alu_add_set(dst, a, b);
  alu.add.op = op;
}


/**
 * This only works if imma == immb (test here internally)
 * The syntax, however, allows this.
 */
Mnemonic::Mnemonic(v3d_qpu_add_op op, Location const &dst, SmallImm const &a, SmallImm const &b) {
  init(NOP);
  if (!alu_add_set(dst, a, b)) throw Exception("Can not use two different small immediates in an instruction");
  alu.add.op = op;
}


Mnemonic &Mnemonic::nop() {
  m_doing_add = false;
  // With normal usage, the mul-part is already nop
  return *this;
}


Mnemonic &Mnemonic::pushz() { set_pf(V3D_QPU_PF_PUSHZ); return *this; }
Mnemonic &Mnemonic::pushc() { set_pf(V3D_QPU_PF_PUSHC); return *this; }
Mnemonic &Mnemonic::pushn() { set_pf(V3D_QPU_PF_PUSHN); return *this; }

Mnemonic &Mnemonic::thrsw()   { sig.thrsw   = true; return *this; }
Mnemonic &Mnemonic::ldvary()  { sig.ldvary  = true; return *this; }
Mnemonic &Mnemonic::ldunif()  { sig.ldunif  = true; return *this; }
Mnemonic &Mnemonic::ldunifa() { sig.ldunifa = true; return *this; }

Mnemonic &Mnemonic::ldunifarf(Location const &loc) {
  sig.ldunifarf = true;

  sig_magic = !loc.is_rf();
  sig_addr = loc.to_waddr();
  return *this;
}


Mnemonic &Mnemonic::ldunifrf(RFAddress const &loc) {
  sig.ldunifrf = true;

  sig_magic = !loc.is_rf();
  sig_addr = loc.to_waddr();
  return *this;
}


Mnemonic &Mnemonic::ldtmu(Register const &reg) {
  sig.ldtmu = true;
  sig_addr  = reg.to_waddr(); 
  sig_magic = true;

  return *this;
}


Mnemonic &Mnemonic::ldvpm()   { sig.ldvpm   = true; return *this; }


void Mnemonic::set_c(v3d_qpu_cond val) {
  if (m_doing_add) {
    flags.ac = val;
  } else {
    flags.mc = val;
  }
}


void Mnemonic::set_uf(v3d_qpu_uf val) {
  if (m_doing_add) {
    flags.auf = val;
  } else {
    flags.muf = val;
  }
}


void Mnemonic::set_pf(v3d_qpu_pf val) {
  if (m_doing_add) {
    flags.apf = val;
  } else {
    flags.mpf = val;
  }
}

Mnemonic &Mnemonic::ifa()   { set_c(V3D_QPU_COND_IFA);  return *this; }
Mnemonic &Mnemonic::ifna()  { set_c(V3D_QPU_COND_IFNA); return *this; }
Mnemonic &Mnemonic::ifb()   { set_c(V3D_QPU_COND_IFB);  return *this; }
Mnemonic &Mnemonic::ifnb()  { set_c(V3D_QPU_COND_IFNB); return *this; }
Mnemonic &Mnemonic::norn()  { set_uf(V3D_QPU_UF_NORN);  return *this; }
Mnemonic &Mnemonic::nornn() { set_uf(V3D_QPU_UF_NORNN); return *this; }
Mnemonic &Mnemonic::norc()  { set_uf(V3D_QPU_UF_NORC);  return *this; }
Mnemonic &Mnemonic::nornc() { set_uf(V3D_QPU_UF_NORNC); return *this; }
Mnemonic &Mnemonic::norz()  { set_uf(V3D_QPU_UF_NORZ);  return *this; }
Mnemonic &Mnemonic::andn()  { set_uf(V3D_QPU_UF_ANDN);  return *this; }
Mnemonic &Mnemonic::andz()  { set_uf(V3D_QPU_UF_ANDZ);  return *this; }
Mnemonic &Mnemonic::andc()  { set_uf(V3D_QPU_UF_ANDC);  return *this; }
Mnemonic &Mnemonic::andnc() { set_uf(V3D_QPU_UF_ANDNC); return *this; }
Mnemonic &Mnemonic::andnn() { set_uf(V3D_QPU_UF_ANDNN); return *this; }

Mnemonic &Mnemonic::a0()     { set_branch_condition(V3D_QPU_BRANCH_COND_A0); return *this; }
Mnemonic &Mnemonic::na0()    { set_branch_condition(V3D_QPU_BRANCH_COND_NA0); return *this; }
Mnemonic &Mnemonic::alla()   { set_branch_condition(V3D_QPU_BRANCH_COND_ALLA); return *this; }
Mnemonic &Mnemonic::allna()  { set_branch_condition(V3D_QPU_BRANCH_COND_ALLNA); return *this; }
Mnemonic &Mnemonic::anya()   { set_branch_condition(V3D_QPU_BRANCH_COND_ANYA); return *this; }
Mnemonic &Mnemonic::anyaq()  { branch.msfign =  V3D_QPU_MSFIGN_Q; return anya(); }
Mnemonic &Mnemonic::anyap()  { branch.msfign =  V3D_QPU_MSFIGN_P; return anya(); }
Mnemonic &Mnemonic::anyna()  { set_branch_condition(V3D_QPU_BRANCH_COND_ANYNA); return *this; }
Mnemonic &Mnemonic::anynaq() { branch.msfign =  V3D_QPU_MSFIGN_Q; return anyna(); }
Mnemonic &Mnemonic::anynap() { branch.msfign =  V3D_QPU_MSFIGN_P; return anyna(); }


Mnemonic &Mnemonic::mov(Location const &dst, SmallImm const &imm) {
  m_doing_add = false;
  alu_mul_set_dst(dst);
  if (!alu_mul_set_imm_a(imm)) assert(false);

  alu.mul.op    = V3D_QPU_M_MOV;
  alu.mul.b     = V3D_QPU_MUX_B;   // Apparently needs to be set also

  return *this;
}


/**
 * Can't consolidate this yet, required for special register vpm
 */
Mnemonic &Mnemonic::mov(uint8_t rf_addr, Register const &reg) {
  m_doing_add = false;

  alu.mul.op    = V3D_QPU_M_MOV;
  alu.mul.a     = reg.to_mux();
  alu.mul.b     = V3D_QPU_MUX_B;
  alu.mul.waddr = rf_addr;

  return *this;
}


Mnemonic &Mnemonic::mov(Location const &loc1, Location const &loc2) {
  m_doing_add = false;
  if (!alu_mul_set(loc1, loc2, loc2)) assert(false); 

  alu.mul.op    = V3D_QPU_M_MOV;
  return *this;
}


Mnemonic &Mnemonic::fmov(Location const &dst,  SmallImm const &imma) {
  m_doing_add = false;
  alu_mul_set_dst(dst);
  if (!alu_mul_set_imm_a(imma)) assert(false);

  alu.mul.op    = V3D_QPU_M_FMOV;  // TODO what's the difference with _MOV? Check
  alu.mul.b     = V3D_QPU_MUX_B;   // Apparently needs to be set also

  return *this;
}


Mnemonic &Mnemonic::add(Location const &dst, Location const &srca, Location const &srcb) {
  m_doing_add = false;
  if (!alu_mul_set(dst, srca, srcb)) assert(false);
  alu.mul.op    = V3D_QPU_M_ADD;
  return *this;
}


Mnemonic &Mnemonic::sub(Location const &dst, Location const &srca, SmallImm const &immb) {
  m_doing_add = false;
  if (!alu_mul_set(dst, srca, immb)) assert(false);
  alu.mul.op    = V3D_QPU_M_SUB;
  return *this;
}


Mnemonic &Mnemonic::sub(Location const &loc1, Location const &loc2, Location const &loc3) {
  m_doing_add = false;
  if (!alu_mul_set(loc1, loc2, loc3)) assert(false);
  alu.mul.op    = V3D_QPU_M_SUB;
  return *this;
}


Mnemonic &Mnemonic::fmul(Location const &loc1, Location const &loc2, Location const &loc3) {
  m_doing_add = false;
  if (!alu_mul_set(loc1, loc2, loc3)) assert(false);

  alu.mul.op    = V3D_QPU_M_FMUL;
  return *this;
}


Mnemonic &Mnemonic::fmul(Location const &loc1, SmallImm imm2, Location const &loc3) {
  m_doing_add = false;
  alu_mul_set(loc1, imm2,  loc3);
  alu.mul.op = V3D_QPU_M_FMUL;
  return *this;
}


Mnemonic &Mnemonic::fmul(Location const &loc1, Location const &loc2, SmallImm const &imm3) {
  m_doing_add = false;
  if (!alu_mul_set(loc1, loc2,  imm3)) assert(false);
  alu.mul.op = V3D_QPU_M_FMUL;
  return *this;
}


Mnemonic &Mnemonic::smul24(Location const &dst, Location const &loca, Location const &locb) {
  m_doing_add = false;
  if (!alu_mul_set(dst, loca, locb)) assert(false);

  alu.mul.op    = V3D_QPU_M_SMUL24;
  return *this;
}


/**
 * NOTE: Added this one myself, not sure if correct
 * TODO verify correctness
 */
Mnemonic &Mnemonic::smul24(Location const &dst, SmallImm const &imma, Location const &locb) {
  m_doing_add = false;
  alu_mul_set(dst, imma, locb);

  alu.mul.op    = V3D_QPU_M_SMUL24;
  return *this;
}


/**
 * TODO verify correctness
 */
Mnemonic &Mnemonic::smul24(Location const &dst, Location const &loca, SmallImm const &immb) {
  m_doing_add = false;
  if (!alu_mul_set(dst, loca, immb)) assert(false);

  alu.mul.op    = V3D_QPU_M_SMUL24;
  return *this;
}


Mnemonic &Mnemonic::vfmul(Location const &rf_addr1, Register const &reg2, Register const &reg3) {
  m_doing_add = false;
  alu_mul_set(rf_addr1, reg2, reg3);

  alu.mul.op = V3D_QPU_M_VFMUL;
  return *this;
}


/**
 * Rotate for mul alu.
 *
 * Rotate only works via the mul ALU.
 *
 * See notes in header comment of rotate overload for add alu above.
 */
Mnemonic &Mnemonic::rotate(Location const &dst, Location const &a, SmallImm const &b) {
  assertq(dst.to_mux()  == V3D_QPU_MUX_R1, "rotate dest can only be r1");
  assertq(a.to_mux() == V3D_QPU_MUX_R0,    "rotate src a can only be r0", true);
  assertq(-15 <= b.val() && b.val() < 16,  "rotate: smallimm must be in proper range");

  m_doing_add = false;
  if (!alu_mul_set(r1, r0, b)) assert(false);

  if (b.val() != 0) {  // Don't bother rotating if there is no rotate
    sig.rotate = true;
  }
  sig.small_imm = false;      // Should *not* be set for rotate
  alu.mul.op = V3D_QPU_M_MOV;

  return *this;
}


/**
 * Rotate for mul alu.
 *
 * See notes in header comment of rotate overload for add alu above.
 */
Mnemonic &Mnemonic::rotate(Location const &dst, Location const &a, Location const &b) {
  assertq(dst.to_mux()  == V3D_QPU_MUX_R1, "rotate dest can only be r1");
  assertq(a.to_mux() == V3D_QPU_MUX_R0,    "rotate src a can only be r0");
  assertq(b.to_mux() == V3D_QPU_MUX_R5,    "rotate src b can only be r5");
  // TODO: check value r5 within range -15,15 inclusive, possible?

  m_doing_add = false;
  if (!alu_mul_set(r1, r0, r5)) assert(false);
  sig.rotate = true;
  alu.mul.op = V3D_QPU_M_MOV;

  return *this;
}


///////////////////////////////////////////////////////////////////////////////
// Registers 
///////////////////////////////////////////////////////////////////////////////

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

Mnemonic nop() { return Mnemonic(); }


Mnemonic tidx(Location const &reg) {
  Mnemonic instr;
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
Mnemonic eidx(Location const &reg) {
  Mnemonic instr;
  instr.alu_add_set_dst(reg);

  instr.alu.add.op    = V3D_QPU_A_EIDX;
  instr.alu.add.a     = V3D_QPU_MUX_R2;
  instr.alu.add.b     = V3D_QPU_MUX_R0;

  return instr;
}


Mnemonic itof(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_ITOF, dst, a, b); }
Mnemonic ftoi(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_FTOIN, dst, a, b); }


Mnemonic shr(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_SHR, dst, a, b); }


Mnemonic mov(Location const &dst, Source const &a)   { return Mnemonic(V3D_QPU_A_OR, dst, a, a); }
Mnemonic mov(Location const &dst, SmallImm const &a) { return Mnemonic(V3D_QPU_A_OR, dst, a, a); }
Mnemonic mov(Location const &dst, Location const &a) { return Mnemonic(V3D_QPU_A_OR, dst, a, a); }

Mnemonic shl(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_SHL, dst, a, b); }
Mnemonic shl(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_SHL, dst, a, b); }
Mnemonic shl(Location const &dst, SmallImm const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_SHL, dst, a, b); }
Mnemonic shl(Location const &dst, SmallImm const &a, Location const &b) { return Mnemonic(V3D_QPU_A_SHL, dst, a, b); }
Mnemonic asr(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_ASR, dst, a, b); }
Mnemonic asr(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_ASR, dst, a, b); }
Mnemonic add(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_ADD, dst, a, b); }
Mnemonic add(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_ADD, dst, a, b); }
Mnemonic add(Location const &dst, SmallImm const &a, Location const &b) { return Mnemonic(V3D_QPU_A_ADD, dst, a, b); }
Mnemonic sub(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_SUB, dst, a, b); }
Mnemonic sub(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_SUB, dst, a, b); }
Mnemonic sub(Location const &dst, SmallImm const &a, Location const &b) { return Mnemonic(V3D_QPU_A_SUB, dst, a, b); }
Mnemonic fsub(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_FSUB, dst, a, b); }
Mnemonic fsub(Location const &dst, SmallImm const &a, Location const &b) { return Mnemonic(V3D_QPU_A_FSUB, dst, a, b); }
Mnemonic fsub(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_FSUB, dst, a, b); }
Mnemonic fadd(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_FADD, dst, a, b); }
Mnemonic fadd(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_FADD, dst, a, b); }
Mnemonic fadd(Location const &dst, SmallImm const &a, Location const &b) { return Mnemonic(V3D_QPU_A_FADD, dst, a, b); }


/**
 * Same as faddf() with mux a and b reversed.
 * The op values are different to distinguish them; in the actual instruction,
 * the operation is actually the same.
 *
 * fmin/fmax have the same relation.
 */
Mnemonic faddnf(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_FADDNF, dst, a, b); }
Mnemonic faddnf(Location const &dst, SmallImm const &a, Location const &b) { return Mnemonic(V3D_QPU_A_FADDNF, dst, a, b); }



///////////////////////////////////////////////////////////////////////////////
// Bitwise Operations
//
// These have prefix 'b' because the expected names are c++ keywords.
//
///////////////////////////////////////////////////////////////////////////////

Mnemonic bor( Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_OR, dst, a, b); }
Mnemonic bor( Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_OR, dst, a, b); }
Mnemonic bor( Location const &dst, SmallImm const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_OR, dst, a, b); }
Mnemonic band(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_AND, dst, a, b); }
Mnemonic band(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_AND, dst, a, b); }
Mnemonic bxor(Location const &dst, Location const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_XOR, dst, a, b); }
Mnemonic bxor(Location const &dst, SmallImm const &a, SmallImm const &b) { return Mnemonic(V3D_QPU_A_XOR, dst, a, b); }

Mnemonic fmax(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_FMAX, dst, a, b); }
Mnemonic fcmp(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_FCMP, dst, a, b); }
Mnemonic vfpack(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_VFPACK, dst, a, b); }
Mnemonic vfmin(Location const &dst, SmallImm const &a, Location const &b) { return Mnemonic(V3D_QPU_A_VFMIN, dst, a, b); }
Mnemonic vfmin(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_VFMIN, dst, a, b); }
Mnemonic min(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_MIN, dst, a, b); }
Mnemonic max(Location const &dst, Location const &a, Location const &b) { return Mnemonic(V3D_QPU_A_MAX, dst, a, b); }


Mnemonic barrierid(v3d_qpu_waddr waddr) {
  Mnemonic instr;

  instr.alu.add.op    = V3D_QPU_A_BARRIERID;
  instr.alu.add.a     = V3D_QPU_MUX_R4;
  instr.alu.add.b     = V3D_QPU_MUX_R2;
  instr.alu.add.waddr = waddr;

  return instr;
}


Mnemonic vpmsetup(Register const &reg2) {
  Mnemonic instr;

  instr.alu.add.op    = V3D_QPU_A_VPMSETUP;
  instr.alu.add.a     = reg2.to_mux();
  instr.alu.add.b     = V3D_QPU_MUX_R3;

  return instr;
}


Mnemonic ffloor(Location const &dst, Source const &srca) {
  Mnemonic instr(V3D_QPU_A_FFLOOR, dst, srca, r1);  // r1 apparently implicit

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
  instr.alu.add.b_unpack = (v3d_qpu_input_unpack) V3D_QPU_A_FFLOOR; // ?? Looks wrong but matches the mesa disasm
#pragma GCC diagnostic pop


  return instr;
}


Mnemonic flpop(RFAddress rf_addr1, RFAddress rf_addr2) {
  Mnemonic instr;

  instr.raddr_a       = rf_addr2.to_waddr();
  instr.alu.add.op    = V3D_QPU_A_FLPOP;
  instr.alu.add.a     = V3D_QPU_MUX_A;
  instr.alu.add.b     = V3D_QPU_MUX_R4;
  instr.alu.add.waddr = rf_addr1.to_waddr();
  instr.alu.add.magic_write = false;

  return instr;
}


Mnemonic fdx(Location const &dst, Location const &srca) {
  Mnemonic instr;
  instr.alu_add_set_dst(dst);
  instr.alu_add_set_reg_a(srca);

  instr.alu.add.op = V3D_QPU_A_FDX;
  return instr;
}


Mnemonic vflb(Location const &dst) {
  Mnemonic instr;
  instr.alu_add_set_dst(dst);

  instr.alu.add.op = V3D_QPU_A_VFLB;
  return instr;
}


Mnemonic tmuwt() {
  Mnemonic instr;
  instr.alu.add.op = V3D_QPU_A_TMUWT;
  return instr;
}


Mnemonic ldvpmg_in(Location const &dst, Location const &a, Location const &b) {
  return Mnemonic(V3D_QPU_A_LDVPMG_IN, dst, a, b);
}


Mnemonic stvpmv(SmallImm const &a, Location const &b) {
  return Mnemonic(V3D_QPU_A_STVPMV, r0, a, b); // r0 is dummy, to align with mesa disasm
}


Mnemonic sampid(Location const &dst) {
  return Mnemonic(V3D_QPU_A_SAMPID, dst, r3, r3);  // Apparently r3s are implicit
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
Mnemonic rotate(Location const &dst, Location const &a, Location const &b) {
  Mnemonic instr;
  return instr.rotate(dst, a, b);
}


/**
 * Rotate.
 *
 * Rotate only works via the mul ALU.
 *
 * See notes in header comment of rotate overload above.
 */
Mnemonic rotate(Location const &dst, Location const &a, SmallImm const &b) {
  Mnemonic instr;
  return instr.rotate(dst, a, b);
}


///////////////////////////////////////////////////////////////////////////////
// Branch instructions
///////////////////////////////////////////////////////////////////////////////

/**
 * Jump relative
 *
 * This creates an unconditionial jump.
 * Add conditions with the associated methods, eg. `na0()`
 *
 * TODO: can we get rid of this in favor of  the override?
 */
Mnemonic branch(int target, int current) {
  Mnemonic instr;
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
Mnemonic branch(int target, bool relative) {
  Mnemonic instr;
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
Mnemonic bb(Location const &loc1) {
  Mnemonic instr;
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


Mnemonic bb(BranchDest const &loc1) {
  Mnemonic instr;
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


Mnemonic bb(uint32_t addr) {
  Mnemonic instr;
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


Mnemonic bu(uint32_t addr, Location const &loc2) {
  //printf("called bu(uint32_t addr, Location const &loc2)\n");

  Mnemonic instr;
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
Mnemonic bu(BranchDest const &loc1, Location const &loc2) {
  //printf("called Mnemonic bu(BranchDest const &loc1, Location const &loc2)\n");

  Mnemonic instr;
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
// SFU instructions - NOT WORKING, they return nothing
//
// Prefix 'b' used to disambiguate, naming collision with std lib functions.
///////////////////////////////////////////////////////////////////////////////

Mnemonic brecip(Location const &dst, Location const &a) { return Mnemonic(V3D_QPU_A_RECIP, dst, a, r5); } // r5 implicit
Mnemonic brsqrt(Location const &dst, Location const &a) { return Mnemonic(V3D_QPU_A_RSQRT, dst, a, r3); } // r3 implicit
Mnemonic brsqrt2(Location const &dst, Location const &a) { return Mnemonic(V3D_QPU_A_RSQRT2, dst, a, r3); } // r3 implicit
Mnemonic bsin(Location const &dst, Location const &a) { return Mnemonic(V3D_QPU_A_SIN, dst, a, a); } // 2nd a implicit 
Mnemonic bexp(Location const &dst, Location const &a) { return Mnemonic(V3D_QPU_A_EXP, dst, a, r4); } // r4 implicit
Mnemonic blog(Location const &dst, Location const &a) { return Mnemonic(V3D_QPU_A_LOG, dst, a, r5); } // r5 implicit


///////////////////////////////////////////////////////////////////////////////
// Aggregated instructions
///////////////////////////////////////////////////////////////////////////////

/**
 * a is a multiple of PI; i.e. a = 0.5 corresponds with PI/2
 *
 * Returns values for -0.5 <= a <= 0.5
 * Anything above is always 1, anything below always -1.
 */
Mnemonics fsin(Location const &dst, Source const &a) {
  Mnemonics ret;

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
