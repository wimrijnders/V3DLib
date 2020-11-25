#include "Snippets.h"
#include "Support/basics.h"

namespace V3DLib {
namespace v3d {
namespace instr {

Instructions set_qpu_id(uint8_t reg_qpu_id) {
	Instructions ret;

	ret << tidx(r0)
			<< bor(rf(reg_qpu_id), r0, r0);  // Actually mov

	ret.front().header("Set QPU id");
	return ret;
}

Instructions set_qpu_num(uint8_t num_qpus, uint8_t reg_qpu_num) {
	Instructions ret;

	if (num_qpus == 1) {
		ret << mov(rf(reg_qpu_num), 0);
	} else if (num_qpus == 8) {
		ret << tidx(r0)
		    << shr(r0, r0, 2)
		    << band(rf(reg_qpu_num), r0, 0b1111);
	} else {
		assert(false);  // num_qpus must be 1 or 8
	}

	ret.front().header("Set number of QPU's");
	return ret;
}


/**
 * TODO Consolidate with calc_offset()
 *
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


/**
 * Determine address offset for address registers.
 *
 * The offset is put in r0.
 * A register file location is also used as a temp storage location.
 *
 * @param reg_qpu_num index in the register file for location to put the qpu num in
 */
Instructions calc_offset(uint8_t num_qpus, uint8_t reg_qpu_num) {
	Instructions ret;

	ret << set_qpu_num(num_qpus, reg_qpu_num);

	ret << shl(r0, rf(reg_qpu_num), 4)
	    << eidx(r1)
	    << add(r0, r0, r1)
	    << shl(r0, r0, 2);


	const char *text = 
		"Determine offset -> r0\n"
		"addr += 4 * (thread_num + 16 * qpu_num)";

	ret.front().comment(text);
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
 * Calculates stride and start address per QPU
 *
 * @param reg_stride rf slot in which to store the stride
 */
Instructions calc_stride( uint8_t num_qpus, uint8_t reg_stride) {
	Instructions ret;

	uint8_t num_qpus_shift = get_shift(num_qpus);

	const char *text = "stride = 4 * 16 * num_qpus";

	ret << mov(rf(reg_stride), 1)
	    << shl(rf(reg_stride), rf(reg_stride), 6 + num_qpus_shift);

	ret.front().header(text);
	return ret;
}


/**
 * An instruction may be passed in to make use of a waiting slot.
 */
Instructions enable_tmu_read(Instr const *last_slot) {
	Instructions ret;

	const char *text = 
		"This single thread switch and two instructions just before the loop are\n"
		"really important for TMU read to achieve a better performance.\n"
		"This also enables TMU read requests without the thread switch signal, and\n"
		"the eight-depth TMU read request queue.";

	ret << nop().thrsw()
	    << nop();

	if (last_slot != nullptr) {
		ret << *last_slot;
	} else {
		ret << nop();
	}

	ret.front().header(text);
	return ret;
}


Instructions sync_tmu() {
	Instructions ret;

	const char *text = 
		"This synchronization is needed between the last TMU operation and the\n"
		"program end with the thread switch just before the main body above.";

	ret << barrierid(syncb).thrsw()
	    << nop()
	    << nop();

	ret.front().header(text);
	return ret;
}


Instructions end_program() {
	Instructions ret;

	ret << nop().thrsw()
	    << nop().thrsw()
	    << nop()
	    << nop()
	    << nop().thrsw()
	    << nop()
	    << nop()
	    << nop();

	ret.front().header("Program tail");
	return ret;
}


}  // instr
}  // v3d
}  // V3DLib

