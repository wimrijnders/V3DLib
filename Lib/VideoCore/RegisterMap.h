#ifdef QPU_MODE

#ifndef _QPULIB_REGISTERMAP_H
#define _QPULIB_REGISTERMAP_H
#include <memory>
#include <stdint.h>

namespace QPULib {

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
	int qpu[MAX_AVAILABLE_QPUS];
};


/**
 * @brief interface for the VideoCore registers.
 *
 * This implementation is far from complete. It only reads
 * two fields from a single register. Regard it as a proof of
 * concept which can be expanded as needed.
 *
 * Implemented as singleton with lazy load, so that it's 
 * not initialized when it's not used.
 */
class RegisterMap {
public:
	RegisterMap(RegisterMap const &) = delete;
	void operator=(RegisterMap const &) = delete;

	~RegisterMap();

	static int numSlices();
	static int numQPUPerSlice();
	static int numTMUPerSlice();
	static int VPMMemorySize();
	static SchedulerRegisterValues SchedulerRegisters();

private:
	RegisterMap();

	volatile uint32_t *m_addr{nullptr};
	unsigned m_size{0};

	static std::unique_ptr<RegisterMap> m_instance;

	uint32_t read(int offset) const;
	static uint32_t readRegister(int offset);

	static RegisterMap *instance();
	static void check_page_align(unsigned addr);
};

}  // namespace QPULib

#endif  // _QPULIB_REGISTERMAP_H

#endif  // QPU_MODE
