#ifndef _V3DLIB_V3D_INSTR_SOURCE_H
#define _V3DLIB_V3D_INSTR_SOURCE_H
#include <memory>
#include "Register.h"
#include "SmallImm.h"
#include "RFAddress.h"
#include "Target/instr/RegOrImm.h"

namespace V3DLib {
namespace v3d {
namespace instr {

class Source {
public:
  Source(V3DLib::RegOrImm const &rhs);
  Source(Register const &rhs);
  Source(Location const &rhs);
  Source(RFAddress const &rhs);
  Source(SmallImm const &rhs);
  Source(int rhs);

  bool is_location() const { return m_is_location; }
  Location const &location() const;
  SmallImm const &small_imm() const;

  bool operator==(Source const &rhs) const;
  bool operator==(Location const &rhs) const;

private:
  bool m_is_location = false;
  std::unique_ptr<Location> m_location;
  SmallImm m_small_imm = 0;
};


inline bool operator==(Location const &lhs, Source const &rhs) { return rhs == lhs; }

}  // namespace instr
}  // namespace v3d
}  // namespace V3DLib


#endif  // _V3DLIB_V3D_INSTR_SOURCE_H
