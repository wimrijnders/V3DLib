#ifndef _V3DLIB_V3D_INSTR_REGISTER_H
#define _V3DLIB_V3D_INSTR_REGISTER_H
#include <string>
#include "Location.h"

namespace V3DLib {
namespace v3d {
namespace instr {

class DestReg {
public:
  DestReg() : m_used(false) {}
  DestReg(uint8_t waddr, bool magic_write) : m_used(true),  m_waddr(waddr), m_magic_write(magic_write) {}

  bool used() const { return m_used; }

  bool operator==(DestReg const &rhs) const {
    if (!m_used || !rhs.m_used) return false;
    return (m_waddr == rhs.m_waddr && m_magic_write == rhs.m_magic_write);
  }

private:
  bool    m_used = false;
  uint8_t m_waddr;
  bool    m_magic_write;
};


class Register : public Location {
public: 
  Register(Register const &rhs) = default;
  Register(const char *name, v3d_qpu_waddr waddr_val);
  Register(const char *name, v3d_qpu_waddr waddr_val, v3d_qpu_mux mux_val, bool is_dest_acc = false);

  v3d_qpu_waddr to_waddr() const override { return m_waddr_val; }
  v3d_qpu_mux to_mux() const override;
  Location *clone() const override { return new Register(*this); }

  Register l() const;
  Register ll() const;
  Register hh() const;
  Register h() const;
  Register abs() const;
  Register swp() const;

  bool is_dest_acc() const { return m_is_dest_acc; }
  std::string const &name() const { return m_name; }

  bool operator==(Location const &rhs) const override;

private:
  std::string   m_name;
  v3d_qpu_waddr m_waddr_val;
  v3d_qpu_mux   m_mux_val;
  bool          m_mux_is_set  = false;
  bool          m_is_dest_acc = false;
};


class BranchDest : public Location {
public: 
  BranchDest(const char *name, v3d_qpu_waddr dest) : m_name(name), m_dest(dest) {}
  Location *clone() const override { return new BranchDest(*this); }

  v3d_qpu_waddr to_waddr() const override { return m_dest; }
  v3d_qpu_mux to_mux() const override;

  bool operator==(Location const &rhs) const override;

private:
  std::string   m_name;
  v3d_qpu_waddr m_dest;
};

}  // instr
}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_REGISTER_H
