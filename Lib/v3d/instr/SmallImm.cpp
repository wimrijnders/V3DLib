#include "SmallImm.h"
#include <stdio.h>
#include <vector>
#include "Support/basics.h"

namespace V3DLib {
namespace v3d {
namespace instr {
namespace {

struct float_encoding {
  float_encoding(float in_val, int in_encoded) : val(in_val), encoded(in_encoded) {}

  float val;
  int encoded;
};


// The rep_value is the hex representation of the IEEE 754 floats (confirmed)
std::vector<float_encoding> float_encodings = {
 {     0,   0x00000000 }, /* 0, same as int 0 */
 {     1,   0x3f800000 }, /* 2.0^0 */
 {     2,   0x40000000 }, /* 2.0^1 */
 {     4,   0x40800000 }, /* 2.0^2 */
 {     8,   0x41000000 }, /* 2.0^3 */
 {    16,   0x41800000 }, /* 2.0^4 */
 {    32,   0x42000000 }, /* 2.0^5 */
 {    64,   0x42800000 }, /* 2.0^6 */
 {   128,   0x43000000 }, /* 2.0^7 */
 {  0.00390625f, 0x3b800000 }, /* 2.0^-8 */
 {  0.0078125f, 0x3c000000 }, /* 2.0^-7 */
 {  0.015625f, 0x3c800000 }, /* 2.0^-6 */
 {  0.03125f, 0x3d000000 }, /* 2.0^-5 */
 {  0.0625f, 0x3d800000 }, /* 2.0^-4 */
 {  0.125f, 0x3e000000 }, /* 2.0^-3 */
 {  0.25f, 0x3e800000 }, /* 2.0^-2 */
 {  0.5f, 0x3f000000 } /* 2.0^-1 */
};


}  // anon namespace

/**
 * @return true if conversion succeeded, false otherwise
 */
bool SmallImm::int_to_opcode_value(int value, int &rep_value) {
  if (-16 <= value && value <= 15) {  // This is the range of legal int values for small imm
    rep_value = (int) value;
    return true;
  }

  return false;
}


/**
 * @return true if conversion succeeded, false otherwise
 */
bool SmallImm::float_to_opcode_value(float value, int &rep_value) {
  bool found_it  = false;

  for (auto &item : float_encodings) {
    if (item.val == value) {
      rep_value = item.encoded;
      found_it = true;
      break;
    }
  }

  return found_it;
}


SmallImm::SmallImm(int val, bool is_val) : m_val(val), m_val_is_set(is_val) {
  if (is_val) pack();
  else m_index = (uint8_t) val;
}


bool SmallImm::is_legal_encoded_value(int value) {
  // Int range
  if (-16 <= value && value <= 15) return true;

  for (auto &item : float_encodings) {
    if (item.encoded == value) {
      return true;
    }
  }

  return false;
}


std::string SmallImm::print_encoded_value(int value) {
  std::string ret;

  // Int range
  if (-16 <= value && value <= 15) ret << value;

  for (auto &item : float_encodings) {
    if (item.encoded == value) {
      ret << item.val;
      break;
    }
  }

  if (ret.empty()) {
    ret << "<Unknown encoded value: " << value << ">";
  }

  return ret;
}


uint8_t SmallImm::to_raddr() const {
  assertq(m_index != 0xff, "Incorrect index value", true);
  return m_index;
}


int SmallImm::val() const {
  assertq(m_val_is_set, "SmallImm::val(): val not set");
  return m_val;
}


bool SmallImm::operator==(SmallImm const &rhs) const {
  assertq(m_index != 0xff, "Incorrect index value", true);
  assertq(rhs.m_index != 0xff, "Incorrect index value rhs", true);
  return m_index == rhs.m_index;
}


void SmallImm::pack() {
  assert(m_val_is_set);
  uint32_t packed_small_immediate;

  if (small_imm_pack(m_val, &packed_small_immediate)) {
    assert(packed_small_immediate <= 0xff);  // to be sure conversion is OK
    m_index = (uint8_t) packed_small_immediate;
  } else {
    printf("SmallImm::pack(): Can not pack value %d\n", m_val);
    breakpoint
    assert(false);
  }
}


SmallImm SmallImm::l() const {
  SmallImm ret(*this);
  ret.m_input_unpack = V3D_QPU_UNPACK_L;
  return ret;
}


SmallImm SmallImm::ff() const {
  SmallImm ret(*this);
  ret.m_input_unpack = V3D_QPU_UNPACK_REPLICATE_32F_16;
  return ret;
}


}  // instr
}  // v3d
}  // V3DLib
