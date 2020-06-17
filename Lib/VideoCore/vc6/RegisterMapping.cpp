// Derived from: https://github.com/Idein/py-videocore6/blob/master/videocore6/v3d.py
//
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "RegisterMapping.h"
#include "readwrite4.h"
#include "../Mailbox.h"  // for mapmem()


namespace {

enum: unsigned {  // NOTE: the pointers are to 4-bit words
	HUB_IDENT1 = 0x0000c >> 2,

  CORE_BASE   = (0xfec04000 + 0x4000) >> 2,
	CORE_IDENT1 = 0x00004 >> 2
};


uint32_t HubField(uint32_t reg, unsigned high, unsigned low) {
  //assert isinstance(register, HubRegister)
  unsigned mask = ((1 << (high - low + 1)) - 1) << low;

	//printf("mask: %08x\n", mask);
	return (reg & mask) >> low;
}

}  // anon namespace


namespace QPULib {
namespace vc6 {


//
//  Adjusted from: RegisterMap::RegisterMap()
//
void RegisterMapping::init() {
	// Original TODO:
	//
  // XXX: Should use bcm_host_get_peripheral_address for the base address
  // on userland, and consult /proc/device-tree/__symbols__/v3d and then
  // /proc/device-tree/v3dbus/v3d@7ec04000/{reg-names,reg} for the offsets
  // in the future.
  unsigned offset = 0xfec00000;

  m_addr = (uint32_t *) mapmem(offset, m_size);

	for (unsigned core = 0; core < m_ncores; ++core) {
    offset = 0xfec04000 + 0x4000 * core;
    map_cores[core] = (uint32_t *) mapmem(offset, m_size);
  }
}


RegisterMapping::~RegisterMapping() {
	unmapmem((void *) m_addr, m_size);

	for (unsigned core = 0; core < m_ncores; ++core) {
		unmapmem((void *) map_cores[core], m_size);
  }
}


unsigned RegisterMapping::num_cores() {
	unsigned const HUB_IDENT1_NCORES = HubField(m_addr[HUB_IDENT1], 11, 8);

	return HUB_IDENT1_NCORES;
}


RegisterMapping::CoreInfo RegisterMapping::core_info(unsigned core_index) {
	CoreInfo ret;
	ret.index = core_index;

	uint32_t val  = *(map_cores[core_index] + CORE_IDENT1);

	unsigned CORE_IDENT1_VPM_SIZE = HubField(val, 31, 28);
	unsigned CORE_IDENT1_NSEM = HubField(val, 23, 16);
	unsigned CORE_IDENT1_NSLC = HubField(val, 7, 4);  // Num slices
	unsigned CORE_IDENT1_REV = HubField(val, 3, 0);

	// Following values appear to be *per slice*:
	// Source: https://github.com/torvalds/linux/blob/master/drivers/gpu/drm/v3d/v3d_debugfs.c#L151
	unsigned CORE_IDENT1_NTMU = HubField(val, 15, 12);
	unsigned CORE_IDENT1_QUPS = HubField(val, 11, 8);

	unsigned nslc = CORE_IDENT1_NSLC;	

	ret.vpm_size  = CORE_IDENT1_VPM_SIZE;
	ret.num_slice = nslc;
	ret.num_tmu   = CORE_IDENT1_NTMU * nslc;
	ret.num_qpu   = CORE_IDENT1_QUPS * nslc;  // Hoping that this indeed means QPUS

	return ret;
}

}  // vc6
}  // QPULib
