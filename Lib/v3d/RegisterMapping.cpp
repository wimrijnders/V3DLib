// Derived from:
//
// - https://github.com/Idein/py-videocore6/blob/master/videocore6/v3d.py
// - https://github.com/torvalds/linux/blob/d15be546031cf65a0fc34879beca02fd90fe7ac7/drivers/gpu/drm/v3d/v3d_debugfs.c#L127
//
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "RegisterMapping.h"
#include "Support/debug.h"
#include "../vc4/Mailbox.h"  // for mapmem()


namespace {

enum: unsigned {  // NOTE: the pointers are to 4-bit words
	HUB_IDENT1 = 0x0000c >> 2,

	V3D_HUB_IDENT0 = 0x00008 >> 2,
	V3D_HUB_IDENT1 = 0x0000c >> 2,
	V3D_HUB_IDENT2 = 0x00010 >> 2,
	V3D_HUB_IDENT3 = 0x00014 >> 2,

	V3D_PCTR_0_PCTR0  = 0x00680 >> 2,
	V3D_PCTR_0_PCTR31 = 0x006fc >> 2,

	V3D_GMP_STATUS = 0x00800 >> 2,
	V3D_CSD_STATUS = 0x00900 >> 2,
	V3D_ERR_FDBG0  = 0x00f04 >> 2,
	V3D_ERR_FDBGB  = 0x00f08 >> 2,
	V3D_ERR_FDBGR  = 0x00f0c >> 2,
	V3D_ERR_FDBGS  = 0x00f10 >> 2,
	V3D_ERR_STAT   = 0x00f20 >> 2,

	// Comment in the linux driver:  "Per-MMU registers"
	// This implies that there could be more MMU registers, if there are more MMu's

  V3D_MMUC_CONTROL = 0x01000 >> 2,
	V3D_MMU_CTL      = 0x01200 >> 2,

	// Other non-register-address constants
  CORE_BASE       = (0xfec04000 + 0x4000) >> 2,
	CORE_IDENT0     = 0x00000,
	CORE_IDENT1     = 0x00004 >> 2,
	CORE_IDENT2     = 0x00008 >> 2,
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


RegisterMapping::Stats RegisterMapping::stats() {
	Stats ret;


	for (int i = 0; i < Stats::NUM_COUNTERS; ++i) {
		ret.counters[i] = m_addr[V3D_PCTR_0_PCTR0];
	}
	assert((V3D_PCTR_0_PCTR0 + Stats::NUM_COUNTERS -1) ==  V3D_PCTR_0_PCTR31);

	ret.gmp_status   =  m_addr[V3D_GMP_STATUS];
	ret.csd_status   =  m_addr[V3D_CSD_STATUS];
	ret.fdbg0        =  m_addr[V3D_ERR_FDBG0];
	ret.fdbgb        =  m_addr[V3D_ERR_FDBGB];
	ret.fdbgr        =  m_addr[V3D_ERR_FDBGR];
	ret.fdbgs        =  m_addr[V3D_ERR_FDBGS];
	ret.stat         =  m_addr[V3D_ERR_STAT];
	ret.mmuc_control =  m_addr[V3D_MMUC_CONTROL];  // This might possibly be a write-only register
	ret.mmu_ctl      =  m_addr[V3D_MMU_CTL];

	int V3D_MMU_CTL_CAP_EXCEEDED              = 27;
	int V3D_MMU_CTL_CAP_EXCEEDED_ABORT        = 26;
	int V3D_MMU_CTL_CAP_EXCEEDED_INT          = 25;
	int V3D_MMU_CTL_CAP_EXCEEDED_EXCEPTION    = 24;
	int V3D_MMU_CTL_PT_INVALID                = 20;
	int V3D_MMU_CTL_PT_INVALID_ABORT          = 19;
	int V3D_MMU_CTL_PT_INVALID_INT            = 18;
	int V3D_MMU_CTL_PT_INVALID_EXCEPTION      = 17;
	int V3D_MMU_CTL_PT_INVALID_ENABLE         = 16;
	int V3D_MMU_CTL_WRITE_VIOLATION           = 12;
	int V3D_MMU_CTL_WRITE_VIOLATION_ABORT     = 11;
	int V3D_MMU_CTL_WRITE_VIOLATION_INT       = 10;
	int V3D_MMU_CTL_WRITE_VIOLATION_EXCEPTION = 9;
	int V3D_MMU_CTL_TLB_CLEARING              = 7;
	int V3D_MMU_CTL_TLB_STATS_CLEAR           = 3;
	int V3D_MMU_CTL_TLB_CLEAR                 = 2;
	int V3D_MMU_CTL_TLB_STATS_ENABLE          = 1;
	int V3D_MMU_CTL_ENABLE                    = 0;

	auto &r = ret.mmu_ctl_fields;
	r.cap_exceeded              = (ret.mmu_ctl & (1 << V3D_MMU_CTL_CAP_EXCEEDED)) != 0;
	r.cap_exceeded_abort        = (ret.mmu_ctl & (1 << V3D_MMU_CTL_CAP_EXCEEDED_ABORT)) != 0;
	r.cap_exceeded_int          = (ret.mmu_ctl & (1 << V3D_MMU_CTL_CAP_EXCEEDED_INT)) != 0;
	r.cap_exceeded_exception    = (ret.mmu_ctl & (1 << V3D_MMU_CTL_CAP_EXCEEDED_EXCEPTION)) != 0;
	r.pt_invalid                = (ret.mmu_ctl & (1 << V3D_MMU_CTL_PT_INVALID)) != 0;
	r.pt_invalid_abort          = (ret.mmu_ctl & (1 << V3D_MMU_CTL_PT_INVALID_ABORT)) != 0;
	r.pt_invalid_int            = (ret.mmu_ctl & (1 << V3D_MMU_CTL_PT_INVALID_INT)) != 0;
	r.pt_invalid_exception      = (ret.mmu_ctl & (1 << V3D_MMU_CTL_PT_INVALID_EXCEPTION)) != 0;
	r.pt_invalid_enable         = (ret.mmu_ctl & (1 << V3D_MMU_CTL_PT_INVALID_ENABLE)) != 0;
	r.write_violation           = (ret.mmu_ctl & (1 << V3D_MMU_CTL_WRITE_VIOLATION)) != 0;
	r.write_violation_abort     = (ret.mmu_ctl & (1 << V3D_MMU_CTL_WRITE_VIOLATION_ABORT)) != 0;
	r.write_violation_int       = (ret.mmu_ctl & (1 << V3D_MMU_CTL_WRITE_VIOLATION_INT)) != 0;
	r.write_violation_exception = (ret.mmu_ctl & (1 << V3D_MMU_CTL_WRITE_VIOLATION_EXCEPTION)) != 0;
	r.tlb_clearing              = (ret.mmu_ctl & (1 << V3D_MMU_CTL_TLB_CLEARING)) != 0;
	r.tlb_stats_clear           = (ret.mmu_ctl & (1 << V3D_MMU_CTL_TLB_STATS_CLEAR)) != 0;
	r.tlb_clear                 = (ret.mmu_ctl & (1 << V3D_MMU_CTL_TLB_CLEAR)) != 0;
	r.tlb_stats_enable          = (ret.mmu_ctl & (1 << V3D_MMU_CTL_TLB_STATS_ENABLE)) != 0;
	r.enable                    = (ret.mmu_ctl & (1 << V3D_MMU_CTL_ENABLE)) != 0;

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
