#ifndef _V3DLIB_TARGET_INSTR_REG_H_
#define _V3DLIB_TARGET_INSTR_REG_H_
#include "Source/Var.h"

namespace V3DLib {

// ============================================================================
// Registers
// ============================================================================

typedef int RegId;

// Different kinds of registers
enum RegTag {
    REG_A           // In register file A (0..31)
  , REG_B           // In register file B (0..31)
  , ACC             // Accumulator register
  , SPECIAL         // Special register
  , NONE            // No read/write
  , TMP_A           // Used in intermediate code
  , TMP_B           // Used in intermediate code
};


// Special registers
enum Special {
  // Read-only
  SPECIAL_UNIFORM,
  SPECIAL_ELEM_NUM,
  SPECIAL_QPU_NUM,

  // DMA Read-only
  SPECIAL_VPM_READ,
  SPECIAL_DMA_ST_WAIT,
  SPECIAL_DMA_LD_WAIT,

  // DMA Write-only
  SPECIAL_RD_SETUP,
  SPECIAL_WR_SETUP,
  SPECIAL_HOST_INT,
  SPECIAL_DMA_LD_ADDR,

  // DMA registers reused for v3d TMU
  // Write-only
  SPECIAL_DMA_ST_ADDR,
  SPECIAL_VPM_WRITE,
  SPECIAL_TMU0_S,

  // SFU registers
  SPECIAL_SFU_RECIP,
  SPECIAL_SFU_RECIPSQRT,
  SPECIAL_SFU_EXP,
  SPECIAL_SFU_LOG
};


struct Reg {
  RegTag tag;   // What kind of register is it?
  RegId regId;  // Register identifier

  bool isUniformPtr = false;

  Reg() = default;
  Reg(RegTag in_tag, RegId in_regId) : tag(in_tag), regId(in_regId) {}

  bool operator==(Reg const &rhs) const { return tag == rhs.tag && regId == rhs.regId; }
  bool operator!=(Reg const &rhs) const { return !(*this == rhs); }
  bool operator<(Reg const &rhs) const;
  bool is_rf_reg() const { return tag == REG_A || tag == REG_B; }

  RegTag regfile() const; 

  std::string dump() const;
};


Reg srcReg(Var v);
Reg dstReg(Var v);

bool is_dma_only_register(Reg const &reg);

}  // namespace V3DLib


#endif  // _V3DLIB_TARGET_INSTR_REG_H_
