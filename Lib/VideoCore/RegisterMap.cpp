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


void RegisterMap::write(int offset, uint32_t value) {
	m_addr[V3D_BASE + offset] = value;
}


/**
 * @brief Convenience function for getting a register value.
 *
 * This avoids having to use `instance()->` for every read access.
 */
uint32_t RegisterMap::readRegister(int offset) {
	return instance()->read(offset);
}


void RegisterMap::writeRegister(int offset, uint32_t value) {
	return instance()->write(offset, value);
}


int RegisterMap::TechnologyVersion() {
	uint32_t reg = readRegister(V3D_IDENT0);
	char buf[4];
	const char *ident = "V3D";

	int ret = (reg >> 24) & 0xf;
	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8)  & 0xff;
	buf[2] = (reg >> 16)  & 0xff;
	buf[3] = '\0';
	
	if (strncmp(ident, buf, 3)) {
		printf("Id string is not the expected 'V3D'!\n");
	} else {
		printf("Id string checks out\n");
	}

	return ret;
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
 * @brief Read the scheduler register values for all QPU's.
 *
 * This reads both scheduler registers.
 *
 * *NOTE:*  The scheduler registers are read/write
 *
 * @return struct with 'do not use' values for all possible values
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


/**
 * @brief Write scheduler register values for selected QPU's.
 *
 * This sets values in both scheduler registers.
 *
 * **NOTE:**  The scheduler registers are read/write
 *
 * **TODO:** Not tested yet!
 */
void RegisterMap::SchedulerRegisters(SchedulerRegisterValues values) {
	uint32_t reg0 = readRegister(V3D_SQRSV0);
	uint32_t reg1 = readRegister(V3D_SQRSV1);
	uint32_t new_reg0 = reg0;
	uint32_t new_reg1 = reg1;

	for (int i = 0; i < MAX_AVAILABLE_QPUS/2; ++i) {
		int offset = 4*i;

		if (values.set_qpu[i]) {
			new_reg0 = (reg0 & ~(0x0f << offset)) | (values.qpu[i] << offset);
		}
	}

	for (int i = MAX_AVAILABLE_QPUS/2; i < MAX_AVAILABLE_QPUS; ++i) {
		int offset = 4*i - MAX_AVAILABLE_QPUS/2;

		if (values.set_qpu[i]) {
			new_reg1 = (reg1 & ~(0x0f << offset)) | (values.qpu[i] << offset);
		}
	}

	if (reg0 != new_reg0) {  // Value reg0 changed
		writeRegister(V3D_SQRSV0, new_reg0);
	}

	if (reg1 != new_reg1) {  // Value reg1 changed
		writeRegister(V3D_SQRSV1, new_reg1);
	}
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
