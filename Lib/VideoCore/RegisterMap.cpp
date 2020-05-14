#ifdef QPU_MODE

#include "RegisterMap.h"
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <bcm_host.h>
#include "Mailbox.h"  // for mapmem()

namespace QPULib {

enum {
	V3D_BASE = (0xc00000 >> 2),
	V3D_IDENT0 = 0,
	V3D_IDENT1,
	V3D_IDENT2,
	V3D_SQRSV0 = (0x00410 >> 2 ),  // Scheduler Register QPUS 0-7
	V3D_SQRSV1                     // Scheduler Register QPUS 8-15
};

std::unique_ptr<RegisterMap> RegisterMap::m_instance;


RegisterMap::RegisterMap() {
	bcm_host_init();
	unsigned addr = bcm_host_get_peripheral_address();
	m_size = bcm_host_get_peripheral_size();

	check_page_align(addr);

	// Following succeeds if it returns.
	m_addr = (uint32_t *) mapmem(addr, m_size);
	assert(m_addr != nullptr);
	// printf("init address: %08X, size: %u\n", m_addr, m_size);
}


RegisterMap::~RegisterMap() {
	// printf("Closing down register map\n");
	unmapmem((void *) m_addr, m_size);
	bcm_host_deinit();
}


/**
 * @brief Get the 32-bit value at the given offset in the map
 */
uint32_t RegisterMap::read(int offset) const {
	return m_addr[V3D_BASE + offset];
}


/**
 * @brief Convenience function for getting a register value.
 *
 * This avoids having to use `instance()->` for every read access.
 */
uint32_t RegisterMap::readRegister(int offset) {
	return instance()->read(V3D_IDENT1);
}


int RegisterMap::numSlices() {
	uint32_t reg = readRegister(V3D_IDENT1);
	// printf("reg V3D_IDENT1: %08X\n", reg);

	return (reg >> 4) & 0xf;
}


int RegisterMap::numQPUPerSlice() {
	return (readRegister(V3D_IDENT1) >> 8) & 0xf;
}


int RegisterMap::numTMUPerSlice() {
	return (readRegister(V3D_IDENT1) >> 12) & 0xf;
}


/**
 * @return VPM memory size in KB
 */
int RegisterMap::VPMMemorySize() {
	return (readRegister(V3D_IDENT1) >> 28) & 0xf;
}


/**
 * @brief Get the scheduler register values for all QPU's
 *
 * This reads both scheduler registers.
 * Note that these values are read/write.
 *
 * @brief struct with 'do not use' values for all possible values
 */
SchedulerRegisterValues RegisterMap::SchedulerRegisters() {
	SchedulerRegisterValues ret;

	uint32_t reg0 = readRegister(V3D_SQRSV0);
	uint32_t reg1 = readRegister(V3D_SQRSV1);

	ret.qpu[ 0] = reg0       & 0x0f;
	ret.qpu[ 1] = reg0 >>  4 & 0x0f;
	ret.qpu[ 2] = reg0 >>  8 & 0x0f;
	ret.qpu[ 3] = reg0 >> 12 & 0x0f;
	ret.qpu[ 4] = reg0 >> 16 & 0x0f;
	ret.qpu[ 5] = reg0 >> 20 & 0x0f;
	ret.qpu[ 6] = reg0 >> 24 & 0x0f;
	ret.qpu[ 7] = reg0 >> 28 & 0x0f;

	ret.qpu[ 8] = reg1       & 0x0f;
	ret.qpu[ 9] = reg1 >>  4 & 0x0f;
	ret.qpu[10] = reg1 >>  8 & 0x0f;
	ret.qpu[11] = reg1 >> 12 & 0x0f;
	ret.qpu[12] = reg1 >> 16 & 0x0f;
	ret.qpu[13] = reg1 >> 20 & 0x0f;
	ret.qpu[14] = reg1 >> 24 & 0x0f;
	ret.qpu[16] = reg1 >> 28 & 0x0f;

	return ret;
}


RegisterMap *RegisterMap::instance() {
	if (m_instance.get() == nullptr) {
		m_instance.reset(new RegisterMap);
	}

	return m_instance.get();
}


void RegisterMap::check_page_align(unsigned addr) {
	long pagesize = sysconf(_SC_PAGESIZE);

	if (pagesize <= 0) {
		fprintf(stderr, "error: sysconf: %s\n", strerror(errno));
		exit(-1);
	}

	if (addr & (((unsigned) pagesize) - 1)) {
		fprintf(stderr, "error: peripheral address is not aligned to page size\n");
		exit(-1);
	}
}

}  // namespace QPULib

#endif  // QPU_MODE
