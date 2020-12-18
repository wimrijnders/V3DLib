// Derived from:
//
// - https://github.com/Idein/py-videocore6/blob/master/videocore6/v3d.py
// - https://github.com/torvalds/linux/blob/d15be546031cf65a0fc34879beca02fd90fe7ac7/drivers/gpu/drm/v3d/v3d_debugfs.c#L127
//
// --------------------------------
// Kernel code:
// ============
//
// **NOTE:** https://github.com/torvalds/linux is much faster than https://gitlab.freedesktop.org.
//           However, it only contains branch 'master'.
//
// - Branch master, <= lima4.17-rc7: no 'drivers/GPU/v3d'
// - Branch >= lima4.18-rc4:  'gca' in v3d_drv.c
//
///////////////////////////////////////////////////////////////////////////////
#include "RegisterMapping.h"
#include <stdio.h>
#include <unistd.h>  // usleep
#include <memory>
#include "../vc4/Mailbox.h"  // for mapmem()
#include "Support/debug.h"


namespace {

enum: unsigned {  // NOTE: the pointers are to 4-bit words
	V3D_HUB_AXICFG = 0x00000,
	//V3D_HUB_AXICFG_MAX_LEN_MASK                   V3D_MASK(3, 0)
	//V3D_HUB_AXICFG_MAX_LEN_SHIFT                  0
	V3D_HUB_UIFCFG = 0x00004 >> 2,

	V3D_HUB_IDENT0 = 0x00008 >> 2,
	V3D_HUB_IDENT1 = 0x0000c >> 2,
	V3D_HUB_IDENT2 = 0x00010 >> 2,
	V3D_HUB_IDENT3 = 0x00014 >> 2,

	V3D_CTL_L2TFLSTA = 0x00034 >>2,
	V3D_CTL_L2TFLEND = 0x00038 >> 2,

	V3D_HUB_INT_STS     = 0x00050 >> 2,
	V3D_HUB_INT_SET     = 0x00054 >> 2,
	V3D_HUB_INT_CLR     = 0x00058 >> 2,
	V3D_HUB_INT_MSK_STS = 0x0005c >> 2,
	V3D_HUB_INT_MSK_SET = 0x00060 >> 2,
	V3D_HUB_INT_MSK_CLR = 0x00064 >> 2,
	V3D_HUB_INT_MMU_WRV = 1 << 5, // BIT(5)
	V3D_HUB_INT_MMU_PTI = 1 << 4, // BIT(4)
	V3D_HUB_INT_MMU_CAP = 1 << 3, // BIT(3)
	V3D_HUB_INT_MSO     = 1 << 2, // BIT(2)
	V3D_HUB_INT_TFUC    = 1 << 1, // BIT(1)
	V3D_HUB_INT_TFUF    = 1,      // BIT(0)

	V3D_TOP_GR_BRIDGE_REVISION = 0x00000,
	//V3D_TOP_GR_BRIDGE_MAJOR_MASK                  V3D_MASK(15, 8)
	//V3D_TOP_GR_BRIDGE_MAJOR_SHIFT                 8
	//V3D_TOP_GR_BRIDGE_MINOR_MASK                  V3D_MASK(7, 0)
	//V3D_TOP_GR_BRIDGE_MINOR_SHIFT                 0

	V3D_TOP_GR_BRIDGE_SW_INIT_0 = 0x00008,
	V3D_TOP_GR_BRIDGE_SW_INIT_0_V3D_CLK_108_SW_INIT = 1, //  BIT(0)
	V3D_TOP_GR_BRIDGE_SW_INIT_1 = 0x0000c,
	V3D_TOP_GR_BRIDGE_SW_INIT_1_V3D_CLK_108_SW_INIT = 1, // BIT(0)

	V3D_CTL_INT_STS     = 0x00050 >> 2,
	V3D_CTL_INT_SET     = 0x00054 >> 2,
	V3D_CTL_INT_CLR     = 0x00058 >> 2,
	V3D_CTL_INT_MSK_STS = 0x0005c >> 2,
	V3D_CTL_INT_MSK_SET = 0x00060 >> 2,
	V3D_CTL_INT_MSK_CLR = 0x00064 >> 2,
//	V3D_INT_QPU_MASK    = V3D_MASK(27, 16),
	V3D_INT_QPU_SHIFT   = 16,
	V3D_INT_CSDDONE     = 1 << 7, //BIT(7)
	V3D_INT_PCTR        = 1 << 6, // BIT(6)
	V3D_INT_GMPV        = 1 << 5, // BIT(5)
	V3D_INT_TRFB        = 1 << 4, // BIT(4)
	V3D_INT_SPILLUSE    = 1 << 3, // BIT(3)
	V3D_INT_OUTOMEM     = 1 << 2, // BIT(2)
	V3D_INT_FLDONE      = 1 << 1, // BIT(1)
	V3D_INT_FRDONE      = 1,      // BIT(0)

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
	V3D_MMUC_CONTROL_CLEAR    = 1 << 3,  // BIT(3)
	V3D_MMUC_CONTROL_FLUSHING = 1 << 2,  // BIT(2)
	V3D_MMUC_CONTROL_FLUSH    = 1 << 1,  // BIT(1)
	V3D_MMUC_CONTROL_ENABLE   = 1,       // BIT(0)

	V3D_MMU_CTL      = 0x01200 >> 2,
	V3D_MMU_CTL_CAP_EXCEEDED              = 1 << 27,
	V3D_MMU_CTL_CAP_EXCEEDED_ABORT        = 1 << 26,
	V3D_MMU_CTL_CAP_EXCEEDED_INT          = 1 << 25,
	V3D_MMU_CTL_CAP_EXCEEDED_EXCEPTION    = 1 << 24,
	V3D_MMU_CTL_PT_INVALID                = 1 << 20,
	V3D_MMU_CTL_PT_INVALID_ABORT          = 1 << 19,
	V3D_MMU_CTL_PT_INVALID_INT            = 1 << 18,
	V3D_MMU_CTL_PT_INVALID_EXCEPTION      = 1 << 17,
	V3D_MMU_CTL_PT_INVALID_ENABLE         = 1 << 16,
	V3D_MMU_CTL_WRITE_VIOLATION           = 1 << 12,
	V3D_MMU_CTL_WRITE_VIOLATION_ABORT     = 1 << 11,
	V3D_MMU_CTL_WRITE_VIOLATION_INT       = 1 << 10,
	V3D_MMU_CTL_WRITE_VIOLATION_EXCEPTION = 1 << 9,
	V3D_MMU_CTL_TLB_CLEARING              = 1 << 7,
	V3D_MMU_CTL_TLB_STATS_CLEAR           = 1 << 3,
	V3D_MMU_CTL_TLB_CLEAR                 = 1 << 2,
	V3D_MMU_CTL_TLB_STATS_ENABLE          = 1 << 1,
	V3D_MMU_CTL_ENABLE                    = 1,

	V3D_MMU_PT_PA_BASE = 0x01204 >> 2,
	V3D_MMU_HIT        = 0x01208 >> 2,
	V3D_MMU_MISSES     = 0x0120c >> 2,
	V3D_MMU_STALLS     = 0x01210 >> 2,

	/* Address for illegal PTEs to return */
	V3D_MMU_ILLEGAL_ADDR = 0x01230 >> 2,
	V3D_MMU_ILLEGAL_ADDR_ENABLE = 1u << 31,  // BIT(31)

	//
	// Performance Counters
	//
  CORE_BASE       = (0xfec04000 + 0x4000) >> 2,

	// Offsets relative per core
	CORE_IDENT0     = 0x00000,
	CORE_IDENT1     = 0x00004 >> 2,
	CORE_IDENT2     = 0x00008 >> 2,


	//
	// NOTE: sequences CORE_PCTR_0_SRC_[0-7] and CORE_PCTR_0_PCTR[0-31] made public for perf counters


	//
	// Other non-register-address constants
	//
	V3D_CTL_MISCCFG = 0x00018 >> 2,
	// V3D_CTL_MISCCFG_QRMAXCNT_MASK                 V3D_MASK(3, 1)
	// V3D_CTL_MISCCFG_QRMAXCNT_SHIFT                1
	V3D_MISCCFG_OVRTMUOUT = 1  // BIT(0)
};


unsigned v3d_mask(unsigned high, unsigned low) {
	assert(high >= low);
	return ((1 << (high - low + 1)) - 1) << low;
}

uint32_t HubField(uint32_t reg, unsigned high, unsigned low) {
  //assert isinstance(register, HubRegister)
  unsigned mask = v3d_mask(high, low);

	//printf("mask: %08x\n", mask);
	return (reg & mask) >> low;
}

}  // anon namespace


namespace V3DLib {
namespace v3d {
namespace {

std::unique_ptr<RegisterMapping> _instance;

}  // anon namespace


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
	unsigned const HUB_IDENT1_NCORES = HubField(m_addr[V3D_HUB_IDENT1], 11, 8);

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


	auto &r = ret.mmu_ctl_fields;
	r.cap_exceeded              = (ret.mmu_ctl & V3D_MMU_CTL_CAP_EXCEEDED) != 0;
	r.cap_exceeded_abort        = (ret.mmu_ctl & V3D_MMU_CTL_CAP_EXCEEDED_ABORT) != 0;
	r.cap_exceeded_int          = (ret.mmu_ctl & V3D_MMU_CTL_CAP_EXCEEDED_INT) != 0;
	r.cap_exceeded_exception    = (ret.mmu_ctl & V3D_MMU_CTL_CAP_EXCEEDED_EXCEPTION) != 0;
	r.pt_invalid                = (ret.mmu_ctl & V3D_MMU_CTL_PT_INVALID) != 0;
	r.pt_invalid_abort          = (ret.mmu_ctl & V3D_MMU_CTL_PT_INVALID_ABORT) != 0;
	r.pt_invalid_int            = (ret.mmu_ctl & V3D_MMU_CTL_PT_INVALID_INT) != 0;
	r.pt_invalid_exception      = (ret.mmu_ctl & V3D_MMU_CTL_PT_INVALID_EXCEPTION) != 0;
	r.pt_invalid_enable         = (ret.mmu_ctl & V3D_MMU_CTL_PT_INVALID_ENABLE) != 0;
	r.write_violation           = (ret.mmu_ctl & V3D_MMU_CTL_WRITE_VIOLATION) != 0;
	r.write_violation_abort     = (ret.mmu_ctl & V3D_MMU_CTL_WRITE_VIOLATION_ABORT) != 0;
	r.write_violation_int       = (ret.mmu_ctl & V3D_MMU_CTL_WRITE_VIOLATION_INT) != 0;
	r.write_violation_exception = (ret.mmu_ctl & V3D_MMU_CTL_WRITE_VIOLATION_EXCEPTION) != 0;
	r.tlb_clearing              = (ret.mmu_ctl & V3D_MMU_CTL_TLB_CLEARING) != 0;
	r.tlb_stats_clear           = (ret.mmu_ctl & V3D_MMU_CTL_TLB_STATS_CLEAR) != 0;
	r.tlb_clear                 = (ret.mmu_ctl & V3D_MMU_CTL_TLB_CLEAR) != 0;
	r.tlb_stats_enable          = (ret.mmu_ctl & V3D_MMU_CTL_TLB_STATS_ENABLE) != 0;
	r.enable                    = (ret.mmu_ctl & V3D_MMU_CTL_ENABLE) != 0;

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


uint32_t RegisterMapping::v3d_bridge_read(uint32_t offset) {
	assert(false);  // TODO
	// #define V3D_BRIDGE_READ(offset) readl(v3d->bridge_regs + offset)
	return 0;
}


void RegisterMapping::v3d_bridge_write(uint32_t offset, uint32_t val) {
	assert(false);  // TODO
	// #define V3D_BRIDGE_WRITE(offset, val) writel(val, v3d->bridge_regs + offset)
}


void RegisterMapping::v3d_core_write(int core_index, uint32_t offset, uint32_t val) {
	map_cores[core_index][offset] = val;
}


uint32_t RegisterMapping::core_read(int core_index, uint32_t offset) {
	return map_cores[core_index][offset];
}


uint32_t RegisterMapping::v3d_read(uint32_t offset) {
	return m_addr[offset];
}


void RegisterMapping::v3d_write(uint32_t offset, uint32_t val) {
	m_addr[offset] = val;
}


///////////////////////////////////////////////////////////////////////////////
// GPU reset - Taken from v3d kernel code
//
// What a tarpit this turned out to be.
///////////////////////////////////////////////////////////////////////////////

/* Nanosecond scalar representation for kernel time values */
typedef uint64_t	ktime_t;

//
// Support stuff scavenged from kernel
//

/*
 * Add a ktime_t variable and a scalar nanosecond value.
 * res = kt + nsval:
 */
#define ktime_add_ns(kt, nsval)		((kt) + (nsval))


/**
 * ktime_compare - Compares two ktime_t variables for less, greater or equal
 * @cmp1:	comparable1
 * @cmp2:	comparable2
 *
 * Return: ...
 *   cmp1  < cmp2: return <0
 *   cmp1 == cmp2: return 0
 *   cmp1  > cmp2: return >0
 */
static inline int ktime_compare(const ktime_t cmp1, const ktime_t cmp2)
{
	if (cmp1 < cmp2)
		return -1;
	if (cmp1 > cmp2)
		return 1;
	return 0;
}


/**
 * ktime_after - Compare if a ktime_t value is bigger than another one.
 * @cmp1:	comparable1
 * @cmp2:	comparable2
 *
 * Return: true if cmp1 happened after cmp2.
 */
static inline bool ktime_after(const ktime_t cmp1, const ktime_t cmp2)
{
	return ktime_compare(cmp1, cmp2) > 0;
}


// Couldn't be bothered to figure out following from code
inline void might_sleep() {}


/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")


void usleep_range(unsigned min, unsigned max) {
	// Just plain ignore 2nd param
	usleep(min);
}


uint64_t ktime_get_raw() {
	struct timespec t;

	int ret = clock_gettime(CLOCK_REALTIME, &t);
	assert(ret == 0);

	return (int64_t)(t.tv_sec) * (int64_t)1000000000 + (int64_t)(t.tv_nsec);
}


// Source: drivers/gpu/drm/v3d/v3d_drv.h
/**
 * __wait_for - magic wait macro
 *
 * Macro to help avoid open coding check/wait/timeout patterns. Note that it's
 * important that we check the condition again after having timed out, since the
 * timeout could be due to preemption or similar and we've never had a chance to
 * check the condition before the timeout.
 */
#define __wait_for(OP, COND, US, Wmin, Wmax) ({ \
  const ktime_t end__ = ktime_add_ns(ktime_get_raw(), 1000ll * (US)); \
  long wait__ = (Wmin); /* recommended min for usleep is 10 us */ \
  int ret__;              \
  might_sleep();              \
  for (;;) {              \
    const bool expired__ = ktime_after(ktime_get_raw(), end__); \
    OP;             \
    /* Guarantee COND check prior to timeout */   \
    barrier();            \
    if (COND) {           \
      ret__ = 0;          \
      break;            \
    }             \
    if (expired__) {          \
      ret__ = -ETIMEDOUT;       \
      break;            \
    }             \
    usleep_range(wait__, wait__ * 2);     \
    if (wait__ < (Wmax))          \
      wait__ <<= 1;         \
  }               \
  ret__;                \
})

#define _wait_for(COND, US, Wmin, Wmax) __wait_for(, (COND), (US), (Wmin), \
               (Wmax))

#define wait_for(COND, MS)    _wait_for((COND), (MS) * 1000, 10, 1000)


void RegisterMapping::v3d_reset_v3d() {
	// This confuses me; there is no 'bridge' reg under the device-tree for `v3dbus`
	//
	// Perhaps I'm looking at the code of an unreleased kernel release here.
	// Commenting it out and hoping for the best.

/*
	uint32_t version = v3d_bridge_read(V3D_TOP_GR_BRIDGE_REVISION);
	uint32_t major   = HubField(version, 15, 8);

 	if (major == 2) {
 		v3d_bridge_write(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_TOP_GR_BRIDGE_SW_INIT_0_V3D_CLK_108_SW_INIT);
 		v3d_bridge_write(V3D_TOP_GR_BRIDGE_SW_INIT_0, 0);

 		/ * GFXH-1383: The SW_INIT may cause a stray write to address 0
 		 * of the unit, so reset it to its power-on value here.
 		 * /
 		v3d_write(V3D_HUB_AXICFG, v3d_mask(3, 0));
 	} else {
 		//WARN_ON_ONCE(V3D_GET_FIELD(version, V3D_TOP_GR_BRIDGE_MAJOR) != 7);
 		v3d_bridge_write(V3D_TOP_GR_BRIDGE_SW_INIT_1, V3D_TOP_GR_BRIDGE_SW_INIT_1_V3D_CLK_108_SW_INIT);
 		v3d_bridge_write(V3D_TOP_GR_BRIDGE_SW_INIT_1, 0);
 	}



// NOTE: Following might still be of use (copied from if):
 		/ * GFXH-1383: The SW_INIT may cause a stray write to address 0
 		 * of the unit, so reset it to its power-on value here.
 		 * /
 		v3d_write(V3D_HUB_AXICFG, v3d_mask(3, 0));
*/

	//
 	//v3d_init_hw_state(v3d); -> v3d_init_core(struct v3d_dev *v3d, int core)
	//
	int core = 0;

	// For ver < 4.0 only 
#if 0
	/* Set OVRTMUOUT, which means that the texture sampler uniform
 	 * configuration's tmu output type field is used, instead of
 	 * using the hardware default behavior based on the texture
 	 * type.  If you want the default behavior, you can still put
 	 * "2" in the indirect texture state's output_type field.
 	 */
 	v3d_core_write(core, V3D_CTL_MISCCFG, V3D_MISCCFG_OVRTMUOUT);
#endif


 	/* Whenever we flush the L2T cache, we always want to flush
 	 * the whole thing.
 	 */
 	v3d_core_write(core, V3D_CTL_L2TFLSTA, 0);
 	v3d_core_write(core, V3D_CTL_L2TFLEND, ~0);
}


#define V3D_CORE_IRQS ((uint32_t)(V3D_INT_OUTOMEM |	\
			     V3D_INT_FLDONE |	\
			     V3D_INT_FRDONE |	\
			     V3D_INT_CSDDONE |	\
			     V3D_INT_GMPV))

#define V3D_HUB_IRQS ((uint32_t)(V3D_HUB_INT_MMU_WRV |	\
			    V3D_HUB_INT_MMU_PTI |	\
			    V3D_HUB_INT_MMU_CAP |	\
			    V3D_HUB_INT_TFUC))

void RegisterMapping::v3d_irq_enable() {
	int const num_cores = 1;
	int core;

	/* Enable our set of interrupts, masking out any others. */
	for (core = 0; core < num_cores; core++) {
		v3d_core_write(core, V3D_CTL_INT_MSK_SET, ~V3D_CORE_IRQS);
		v3d_core_write(core, V3D_CTL_INT_MSK_CLR, V3D_CORE_IRQS);
	}

	v3d_write(V3D_HUB_INT_MSK_SET, ~V3D_HUB_IRQS);
	v3d_write(V3D_HUB_INT_MSK_CLR, V3D_HUB_IRQS);
}

#undef V3D_HUB_IRQS
#undef V3D_CORE_IRQS


/**
 * Derived from: https://gitlab.freedesktop.org/lima/linux/-/blob/lima-5.0/drivers/gpu/drm/v3d/v3d_gem.c#L97
 */
void RegisterMapping::reset_v3d() {
	bool do_reset = true;

	// v3d_idle_gca(v3d);  - In code, not called for v3d ver >= 4.1. Pi4 starts with v 4.2
	//                     - v < 4.1 uses device tree reg 'gca', not present in my system

	v3d_reset_v3d();

	//
 	//v3d_mmu_set_page_table(v3d);
	//
	unsigned const V3D_MMU_PAGE_SHIFT = 12;

	// `v3d->pt_addr` is only used for DMA calls, which this library doesn't use for v3d.
	// Therefore, cautiously commenting it out.
	//
	//	v3d_write(V3D_MMU_PT_PA_BASE, v3d->pt_paddr >> V3D_MMU_PAGE_SHIFT);
	//

	v3d_write(V3D_MMU_CTL,
		        V3D_MMU_CTL_ENABLE |
		        V3D_MMU_CTL_PT_INVALID |
		        V3D_MMU_CTL_PT_INVALID_ABORT |
		        V3D_MMU_CTL_WRITE_VIOLATION_ABORT |
		        V3D_MMU_CTL_CAP_EXCEEDED_ABORT);

	// `v3d->mmu_scratch_paddr` only used for DMA.
	//
	//	v3d_write(V3D_MMU_ILLEGAL_ADDR,
	//		        (v3d->mmu_scratch_paddr >> V3D_MMU_PAGE_SHIFT) |
	//		        V3D_MMU_ILLEGAL_ADDR_ENABLE);

	v3d_write(V3D_MMUC_CONTROL, V3D_MMUC_CONTROL_ENABLE);

	//
	//v3d_mmu_flush_all(v3d);
	//
	{
		int ret;

		/* Make sure that another flush isn't already running when we
		 * start this one.
		 */
		ret = wait_for(!(v3d_read(V3D_MMU_CTL) & V3D_MMU_CTL_TLB_CLEARING), 100);
		if (ret)
			error("TLB clear wait idle pre-wait failed");

		v3d_write(V3D_MMU_CTL, v3d_read(V3D_MMU_CTL) | V3D_MMU_CTL_TLB_CLEAR);
		v3d_write(V3D_MMUC_CONTROL, V3D_MMUC_CONTROL_FLUSH | V3D_MMUC_CONTROL_ENABLE);

		ret = wait_for(!(v3d_read(V3D_MMU_CTL) & V3D_MMU_CTL_TLB_CLEARING), 100);
		if (ret) {
			error("TLB clear wait idle failed");
			return;  // ret;
		}

		ret = wait_for(!(v3d_read(V3D_MMUC_CONTROL) & V3D_MMUC_CONTROL_FLUSHING), 100);
		if (ret)
			error("MMUC flush wait idle failed");
	}

	v3d_irq_enable();
}


RegisterMapping &RegisterMapping::instance() {
	if (_instance.get() == nullptr) {
		_instance.reset(new RegisterMapping);
		_instance->init();
	}

	return *_instance.get();
}

}  // v3d
}  // V3DLib
