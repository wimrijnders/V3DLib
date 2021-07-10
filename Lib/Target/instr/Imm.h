#ifndef _V3DLIB_TARGET_INSTR_IMM_H_
#define _V3DLIB_TARGET_INSTR_IMM_H_
#include <string>

namespace V3DLib {

struct Imm {

  // Different kinds of immediate
  enum ImmTag {
    IMM_INT32,    // 32-bit word
    IMM_FLOAT32,  // 32-bit float
    IMM_MASK     // 1 bit per vector element (0 to 0xffff)
  };

  enum {
    INVALID_ENCODING = -17
  };

  Imm() = default;
  Imm(int i);
  Imm(unsigned i);
  Imm(float f);

  ImmTag tag() const { return m_tag; }
  bool   is_int() const { return m_tag == IMM_INT32; }
  bool   is_float() const { return m_tag == IMM_FLOAT32; }
  int    intVal() const;
  int    mask() const;
  float  floatVal() const;
  uint32_t encode() const;
  int encode_imm() const;

  bool is_zero() const;
  bool is_basic() const;
  std::string pretty() const;

  bool operator==(Imm const &rhs) const;
  bool operator!=(Imm const &rhs) const { return !(*this == rhs); }

private:
  ImmTag m_tag      = IMM_INT32;
  int    m_intVal   = 0;
  float  m_floatVal = -1.0f;
};

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_INSTR_IMM_H_
