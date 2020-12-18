#ifdef QPU_MODE

#ifndef _V3DLIB_VC4_REGISTERMAP_H
#define _V3DLIB_VC4_REGISTERMAP_H
#include <stdint.h>

namespace V3DLib {

// Masks for scheduler registers
enum SchedulerMasks : int {
	DO_NOT_USE_FOR_USER_PROGRAMS = 1,
	DO_NOT_USE_FOR_FRAGMENT_SHADERS = 2,
	DO_NOT_USE_FOR_VERTEX_SHADERS = 4,
	DO_NOT_USE_FOR_COORDINATE = 8
};

const int MAX_AVAILABLE_QPUS = 16;

// Data structure for returning scheduler register values.
// Length is max available slots, not the actual number of QPU's
struct SchedulerRegisterValues {
	int  qpu[MAX_AVAILABLE_QPUS];
	bool set_qpu[MAX_AVAILABLE_QPUS];  // Used for write option

	SchedulerRegisterValues() {
		for (int i = 0; i < MAX_AVAILABLE_QPUS; ++i) {
			set_qpu[i] = false;
		}
	}
};


/**
 * @brief interface for the vc4 registers.
 *
 * This implementation is far from complete.
 * The reading of more registers can be added as needed.
 *
 * Implemented as singleton with lazy load, so that it's 
 * not initialized when it's not used.
 */
class RegisterMap {
public:
	enum Index {
		V3D_BASE = (0xc00000 >> 2),
		V3D_IDENT0 = 0,
		V3D_IDENT1,
		V3D_IDENT2,
		V3D_L2CACTL = (0x00020 >> 2),
		V3D_SQRSV0  = (0x00410 >> 2 ), // Scheduler Register QPUS 0-7
		V3D_SQRSV1,                    // Scheduler Register QPUS 8-15

		V3D_CT0CS   = (0x00100 >> 2),  // Control List Executor Thread 0 Control and Status.
		V3D_CT1CS,                     // Control List Executor Thread 0 Control and Status.

		//
		// Performance counter register slots.
		//
		// There are 30 performance counters, but only 16 registers available
		// to access them. You therefore have to map the counters you are
		// interested in to an available slot.
		//
		// There is actually no use in specifying all names fully, except to
		// have the list complete. Internally,  the counter registers are taken as offsets
		// from the first (V3D_PCTR0 and V3D_PCTRS0), so for the list it's enough to specify
		// only that one and set an enum for the end of the list.
		// TODO: perhaps make this so
		//
		// PC: 'Performance Counter' below
		//
	  V3D_PCTRC = (0x00670 >> 2),  // PC Clear     - write only
		V3D_PCTRE,                   // PC Enables   - read/write
		V3D_PCTR0 = (0x00680 >> 2),  // PC Count 0   - read/write
		V3D_PCTRS0,                  // PC Mapping 0 - read/write
		V3D_PCTR1,                   // PC Count 1
		V3D_PCTRS1,                  // PC Mapping 1
		V3D_PCTR2,                   // etc.
		V3D_PCTRS2,
		V3D_PCTR3,
		V3D_PCTRS3,
		V3D_PCTR4,
		V3D_PCTRS4,
		V3D_PCTR5,
		V3D_PCTRS5,
		V3D_PCTR6,
		V3D_PCTRS6,
		V3D_PCTR7,
		V3D_PCTRS7,
		V3D_PCTR8,
		V3D_PCTRS8,
		V3D_PCTR9,
		V3D_PCTRS9,
		V3D_PCTR10,
		V3D_PCTRS10,
		V3D_PCTR11,
		V3D_PCTRS11,
		V3D_PCTR12,
		V3D_PCTRS12,
		V3D_PCTR13,
		V3D_PCTRS13,
		V3D_PCTR14,
		V3D_PCTRS14,
		V3D_PCTR15,
		V3D_PCTRS15
	};


	RegisterMap(RegisterMap const &) = delete;
	void operator=(RegisterMap const &) = delete;

	~RegisterMap();

	static int TechnologyVersion();
	static int numSlices();
	static int numQPUPerSlice();
	static int numTMUPerSlice();
	static int VPMMemorySize();
	static int L2CacheEnabled();

	static SchedulerRegisterValues SchedulerRegisters();
	static void SchedulerRegisters(SchedulerRegisterValues values);
	static void resetAllSchedulerRegisters();

	static uint32_t readRegister(int offset);
	static void writeRegister(int offset, uint32_t value);

	static bool checkThreadErrors();

private:
	RegisterMap();

	volatile uint32_t *m_addr{nullptr};
	unsigned m_size{0};

	uint32_t read(int offset) const;
	void write(int offset, uint32_t value);

	static RegisterMap &instance();
	static void check_page_align(unsigned addr);
	static void checkVersionString(uint32_t reg);
	static uint32_t *adress();
	static unsigned size();
};

}  // namespace V3DLib

#endif  // _V3DLIB_VC4_REGISTERMAP_H
#endif  // QPU_MODE
