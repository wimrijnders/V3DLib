#ifdef QPU_MODE

#include "RegisterMap.h"
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <bcm_host.h>
#include "Support/basics.h"  // fatal()
#include "Mailbox.h"         // for mapmem()
#include "vc4.h"


namespace QPULib {

std::unique_ptr<RegisterMap> RegisterMap::m_instance;


RegisterMap::RegisterMap() {
	enableQPUs();

	bcm_host_init();
	unsigned addr = bcm_host_get_peripheral_address();
	//printf("Peripheral base: %08X\n", addr);

	m_size = bcm_host_get_peripheral_size();

	check_page_align(addr);

	// Following succeeds if it returns.
	m_addr = (uint32_t *) mapmem(addr, m_size);
	assert(m_addr != nullptr);
	//printf("init address: %08X, size: %u\n", m_addr, m_size);

	bcm_host_deinit();
}


RegisterMap::~RegisterMap() {
	// printf("Closing down register map\n");
	unmapmem((void *) m_addr, m_size);
//	bcm_host_deinit();
	disableQPUs();
}


/**
 * @brief Get the 32-bit value of the register at the given offset in the map
 */
uint32_t RegisterMap::read(int offset) const {
	return m_addr[V3D_BASE + offset];
}


/**
 * @brief Set the 32-bit value of the register at the given offset in the map
 */
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


/**
 * @brief Convenience function for setting a register value.
 *
 * This avoids having to use `instance()->` for every write access.
 */
void RegisterMap::writeRegister(int offset, uint32_t value) {
	return instance()->write(offset, value);
}


uint32_t *RegisterMap::adress() {
	return (uint32_t *) instance()->m_addr;
}


unsigned RegisterMap::size() {
	return instance()->m_size;
}


int RegisterMap::TechnologyVersion() {
	uint32_t reg = readRegister(V3D_IDENT0);

	checkVersionString(reg);

	int ret = (reg >> 24) & 0xf;
	return ret;
}


void RegisterMap::checkVersionString(uint32_t  reg) {
	const char *ident = "V3D";
	char buf[4];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8)  & 0xff;
buf[2] = (reg >> 16)  & 0xff;
	buf[3] = '\0';
	
	if (strncmp(ident, buf, 3)) {
		printf("Id string is not the expected 'V3D' but '%s'!\n", buf);
	} else {
		printf("Id string checks out\n");
	}

/*
	uint32_t buf2 = 0;
	buf2 |= (ident[0]);
	buf2 |= (ident[1] <<  8) & 0x00ff00;
	buf2 |= (ident[2] << 16) & 0xff0000;


  // Search the memory area for the given string
	const unsigned STEP = 1000;
	const unsigned MAX = 102400;
	unsigned length = size() >> 2;
	if (length > MAX) length = MAX;

	for (unsigned s = 0; s < length; ++s) {
		if (s % STEP == 0) {
			printf("%u,",s);
		}

		uint32_t val = instance()->m_addr[s];
		if ((val & 0x00ffffff) == buf2) {
			printf("Found '%s' at register: %08X\n", buf,  s);
		}
	}

	printf("\n");
*/
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
 * @brief get the size of the VPM.
 *
 * @return Size of VPM in KB
 */
int RegisterMap::VPMMemorySize() {
	uint32_t reg = readRegister(V3D_IDENT1);
	int value = (reg >> 28) & 0xf;

	if (value == 0) return 16;  // According to reference doc p.97
	return value;
}


int RegisterMap::L2CacheEnabled() {
	return (readRegister(V3D_L2CACTL) & 0x1);
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
			//new_reg0 = (reg0 & ~(0x0f << offset)) | (values.qpu[i] << offset);
			new_reg0 = (0x0f & values.qpu[i]) << offset;
		}
	}

	for (int i = MAX_AVAILABLE_QPUS/2; i < MAX_AVAILABLE_QPUS; ++i) {
		int offset = 4*i - MAX_AVAILABLE_QPUS/2;

		if (values.set_qpu[i]) {
			//new_reg1 = (reg1 & ~(0x0f << offset)) | (values.qpu[i] << offset);
			new_reg1 = (0x0f & values.qpu[i]) << offset;
		}
	}

	if (reg0 != new_reg0) {  // Value reg0 changed
		writeRegister(V3D_SQRSV0, new_reg0);
	}

	if (reg1 != new_reg1) {  // Value reg1 changed
		writeRegister(V3D_SQRSV1, new_reg1);
	}
}


void RegisterMap::resetAllSchedulerRegisters() {
	SchedulerRegisterValues values;

	for (int i = 0; i < MAX_AVAILABLE_QPUS; ++i) {
		values.set_qpu[i] = true;   // write for all QPU's
		values.qpu[i]     = 0;      // allow all program types (eg 0)
	}

	SchedulerRegisters(values);
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
		char buf[64];
		sprintf(buf, "error: sysconf: %s", strerror(errno));
		fatal(buf);
	}

	if (addr & (((unsigned) pagesize) - 1)) {
		fatal("error: peripheral address is not aligned to page size");
	}
}


/**
 * @return true if errors present, false otherwise
 */
bool RegisterMap::checkThreadErrors() {
	uint32_t ERROR_BITMASK = 0x08;	// If set, control thread error

	uint32_t reg0 = readRegister(V3D_CT0CS);
	uint32_t reg1 = readRegister(V3D_CT1CS);

	bool ret = false;

	if (reg0 & ERROR_BITMASK) {
		printf("Control thread error for thread 0\n");
		ret = true;
	}

	if (reg1 & ERROR_BITMASK) {
		printf("Control thread error for thread 1\n");
		ret = true;
	}

	return ret;
}


}  // namespace QPULib

#endif  // QPU_MODE
