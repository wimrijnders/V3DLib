#include "Snippets.h"
#include "Support/basics.h"

namespace V3DLib {
namespace v3d {
namespace instr {

Instructions set_qpu_id(uint8_t reg_qpu_id) {
  Instructions ret;

  ret << tidx(r0).header("Set QPU id")
      << bor(rf(reg_qpu_id), r0, r0);  // Actually mov

  return ret;
}

Instructions set_qpu_num(uint8_t num_qpus, uint8_t reg_qpu_num) {
  Instructions ret;

  if (num_qpus == 1) {
    ret << mov(rf(reg_qpu_num), 0).comment("Using 1 QPU");
  } else if (num_qpus == 8) {
    ret << tidx(r0).header("Set number of QPUs to 8")
        << shr(r0, r0, 2)
        << band(rf(reg_qpu_num), r0, 0b1111);
  } else {
    assert(false);  // num_qpus must be 1 or 8
  }

  return ret;
}


/**
 * source:  https://github.com/Idein/py-videocore6/blob/3c407a2c0a3a0d9d56a5d0953caa7b0a4e92fa89/examples/summation.py#L22
 */
Instructions get_num_qpus(Register const &reg, uint8_t num_qpus) {
  assert(num_qpus == 1 || num_qpus == 8);
  assert(reg.is_dest_acc());

  Instructions ret;

  if (num_qpus == 1) {
    ret << mov(reg, 0);
  } else { //  num_qpus == 8
    breakpoint
    ret << tidx(reg)
        << shr(reg, reg, 2)
        << band(reg, reg, 0b1111);
  }

  return ret;
}


uint8_t get_shift(uint64_t num_qpus) {
  uint8_t ret = 0;

  if (num_qpus == 1) {
    ret = 0;
  } else if (num_qpus == 8) {
    ret = 3;
  } else {
    assert(false);  // num_qpus must be 1 or 8
  }

  return ret;
}


/**
 * An instruction may be passed in to make use of a waiting slot.
 */
Instructions enable_tmu_read(Instr const *last_slot) {
  const char *text = 
    "This single thread switch and two instructions just before the loop are\n"
    "really important for TMU read to achieve a better performance.\n"
    "This also enables TMU read requests without the thread switch signal, and\n"
    "the eight-depth TMU read request queue.";

  Instructions ret;

  ret << nop().thrsw().header(text)
      << nop();

  if (last_slot != nullptr) {
    ret << *last_slot;
  } else {
    ret << nop();
  }

  return ret;
}


Instructions sync_tmu() {
  const char *text = 
    "This synchronization is needed between the last TMU operation and the\n"
    "program end with the thread switch just before the main body above.";

  Instructions ret;

  ret << barrierid(syncb).thrsw().header(text)
      << nop()
      << nop();

  return ret;
}


Instructions end_program() {
  Instructions ret;

  ret << nop().thrsw().header("Program tail")
      << nop().thrsw()
      << nop()
      << nop()
      << nop().thrsw()
      << nop()
      << nop()
      << nop();

  return ret;
}


}  // instr
}  // v3d
}  // V3DLib

