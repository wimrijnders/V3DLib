#include "Var.h"
#include "Support/basics.h"

namespace V3DLib {

static int globalVarId = 0;  // Used for fresh variable generation

Var::Var(VarTag tag, bool is_uniform_ptr) : m_tag(tag), m_is_uniform_ptr(is_uniform_ptr)  {
  assert(!is_uniform_ptr || tag == UNIFORM);
}


bool Var::is_uniform_ptr() const { return m_is_uniform_ptr; }


std::string Var::disp() const {
  std::string ret;

  switch(m_tag) {
    case STANDARD:
      ret << "v" << m_id;
    break;
    case UNIFORM:
      ret << "Uniform";
      if (m_is_uniform_ptr) {
        ret << " Ptr";
      }
    break;
    case QPU_NUM:
      ret << "QPU_NUM";
    break;
    case ELEM_NUM:
      ret << "ELEM_NUM";
    break;
    case VPM_READ:
      ret << "VPM_READ";
    break;
    case VPM_WRITE:
      ret << "VPM_WRITE";
    break;
    case TMU0_ADDR:
      ret << "TMU0_ADDR";
    break;
    case DUMMY:
      ret << "Dummy";
    break;
    default:
      assert(false);
    break;
  }

  return ret;
}


/**
 * Obtain a fresh variable
 * @return a fresh standard variable
 */
Var freshVar() {
  return Var(STANDARD, globalVarId++);
}


/**
 * Returns number of fresh vars used
 */
int getFreshVarCount() {
  return globalVarId;
}


/**
 * Reset fresh variable generator
 */
void resetFreshVarGen(int val) {
  assert(val >= 0);
  globalVarId = val;
}

}  // namespace V3DLib
