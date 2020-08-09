// Derived from:
//
// - https://github.com/Idein/py-videocore6/blob/master/videocore6/v3d.py
// - https://github.com/torvalds/linux/blob/d15be546031cf65a0fc34879beca02fd90fe7ac7/drivers/gpu/drm/v3d/v3d_debugfs.c#L127
//
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "RegisterMapping.h"
#include "../VideoCore/Mailbox.h"  // for mapmem()


namespace {

enum: unsigned {  // NOTE: the pointers are to 4-bit words
	HUB_IDENT1 = 0x0000c >> 2,

	V3D_HUB_IDENT0 = 0x00008 >> 2,
	V3D_HUB_IDENT1 = 0x0000c >> 2,
	V3D_HUB_IDENT2 = 0x00010 >> 2,
	V3D_HUB_IDENT3 = 0x00014 >> 2,

  CORE_BASE   = (0xfec04000 + 0x4000) >> 2,
	CORE_IDENT0 = 0x00000,
	CORE_IDENT1 = 0x00004 >> 2,
	CORE_IDENT2 = 0x00008 >> 2,
	V3D_CTL_MISCCFG = 0x00018 >> 2
};


uint32_t HubField(uint32_t reg, unsigned high, unsigned low) {
  //assert isinstance(register, HubRegister)
  unsigned mask = ((1 << (high - low + 1)) - 1) << low;

	//printf("mask: %08x\n", mask);
	return (reg & mask) >> low;
}

}  // anon namespace


namespace QPULib {
namespace v3d {


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


RegisterMapping::Info RegisterMapping::info() {
	Info ret;

	uint32_t V3D_HUB_IDENT2_WITH_MMU = 1 <<  8;
	uint32_t V3D_HUB_IDENT1_WITH_L3C = 1 << 16;
	uint32_t V3D_HUB_IDENT1_WITH_TFU = 1 << 17;
  uint32_t V3D_HUB_IDENT1_WITH_TSY = 1 << 18;
	uint32_t V3D_HUB_IDENT1_WITH_MSO = 1 << 19;

	uint32_t ident0 = m_addr[V3D_HUB_IDENT0];
	uint32_t ident1 = m_addr[V3D_HUB_IDENT1];
	uint32_t ident2 = m_addr[V3D_HUB_IDENT2];
	uint32_t ident3 = m_addr[V3D_HUB_IDENT3];

	ret.num_cores = num_cores();

	ret.tver    = HubField(ident1,  3, 0);
	ret.rev     = HubField(ident1,  7, 4);
	ret.iprev   = HubField(ident3, 15, 8);
	ret.ipidx   = HubField(ident3,  7, 0);
	ret.mmu     = (ident2 & V3D_HUB_IDENT2_WITH_MMU) != 0;
	ret.tfu     = (ident1 & V3D_HUB_IDENT1_WITH_TFU) != 0;
  ret.tsy     = (ident1 & V3D_HUB_IDENT1_WITH_TSY) != 0;
	ret.mso	    = (ident1 & V3D_HUB_IDENT1_WITH_MSO) != 0;
	ret.l3c     = (ident1 & V3D_HUB_IDENT1_WITH_L3C) != 0;
	ret.l3c_nkb = HubField(ident2, 7, 0);

	return ret;
}


RegisterMapping::CoreInfo RegisterMapping::info_per_core(unsigned core_index) {
	CoreInfo ret;
	ret.index = core_index;

	uint32_t val0     = *(map_cores[core_index] + CORE_IDENT0);
	uint32_t val1     = *(map_cores[core_index] + CORE_IDENT1);
	uint32_t val2     = *(map_cores[core_index] + CORE_IDENT2);
	uint32_t misccfg  = *(map_cores[core_index] + V3D_CTL_MISCCFG);

	unsigned CORE_IDENT0_VER = HubField(val0, 31, 24);

	unsigned CORE_IDENT1_VPM_SIZE = HubField(val1, 31, 28);
	unsigned CORE_IDENT1_NSEM = HubField(val1, 23, 16);
	unsigned CORE_IDENT1_NSLC = HubField(val1, 7, 4);  // Num slices
	unsigned CORE_IDENT1_REV = HubField(val1, 3, 0);

	unsigned CORE_IDENT2_BCG = HubField(val2, 28, 28);

	// Following values appear to be *per slice*:
	// Source: https://github.com/torvalds/linux/blob/master/drivers/gpu/drm/v3d/v3d_debugfs.c#L151
	unsigned CORE_IDENT1_NTMU = HubField(val1, 15, 12);
	unsigned CORE_IDENT1_QUPS = HubField(val1, 11, 8);

	unsigned CORE_MISCCFG_QRMAXCNT  = HubField(misccfg, 3, 1);
	unsigned CORE_MISCCFG_OVRTMUOUT = HubField(misccfg, 0, 0);


	unsigned nslc = CORE_IDENT1_NSLC;	

	ret.ver           = CORE_IDENT0_VER;
	ret.rev           = CORE_IDENT1_REV;
	ret.vpm_size      = CORE_IDENT1_VPM_SIZE;
	ret.num_slice     = nslc;
	ret.num_semaphore = CORE_IDENT1_NSEM;
	ret.num_tmu       = CORE_IDENT1_NTMU * nslc;
	ret.num_qpu       = CORE_IDENT1_QUPS * nslc;
	ret.bcg_int       = (CORE_IDENT2_BCG != 0);
	ret.override_tmu  = (CORE_MISCCFG_OVRTMUOUT != 0);

	return ret;
}

}  // v3d
}  // QPULib
