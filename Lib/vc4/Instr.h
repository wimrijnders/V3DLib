#ifndef _V3DLIB_ENCODE_H_
#define _V3DLIB_ENCODE_H_
#include <stdint.h>
#include "Target/instr/Instr.h"

namespace V3DLib {
namespace vc4 {

class Instr {
public:
  void encode(V3DLib::Instr const &instr);
  uint64_t code() const { return (((uint64_t) high()) << 32) + low(); }

private:
  enum Tag {
    NOP,
    ROT,
    ALU,
    LI,
    BR,
    END,
    SINC,
    SDEC,
    LDTMU  // Always writes to ACC4
  };

  uint32_t cond_add = 0;    // reused for LI, BR
  uint32_t cond_mul = 0;
  uint32_t waddr_add = 39;  // reused for LI
  uint32_t waddr_mul = 39;

  uint32_t addOp  = 0;
  uint32_t mulOp  = 0;
  uint32_t muxa   = 0;
  uint32_t muxb   = 0;
  uint32_t raddra = 39;
  uint32_t raddrb = 0;

  uint32_t li_imm = 0;  // Also used as BR target
  uint32_t sema_id = 0;
  Tag m_tag = NOP;
  uint32_t m_sig = 14;        // 0xe0000000
  uint32_t m_sem_flag = 0;    // TODO research what this is for, only used with SINC/SDEC

  bool m_ws  = false;
  bool m_sf  = false;
  bool m_rel = false;

  void tag(Tag in_tag, bool imm = false);
  void sf(bool val) { assert(m_tag != BR); m_sf = val; }
  void ws(bool val) { assert(m_tag != BR); m_ws = val; }
  void rel(bool val) { assert(m_tag == BR); m_rel = val; }

  void encode_operands(RegOrImm const &srcA, RegOrImm const &srcB);

  uint32_t high() const;
  uint32_t low() const;
};

}  // namespace vc4
}  // namespace V3DLib

#endif  // _V3DLIB_ENCODE_H_
