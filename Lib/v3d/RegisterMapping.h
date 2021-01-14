#ifndef _VC6_REGISTERMAPPING_H
#define _VC6_REGISTERMAPPING_H
#include <stdint.h>


namespace V3DLib {
namespace v3d {

class RegisterMapping {
public:
	enum {
		//
		// Values made public for PerformanceCounters
		//
		CORE_PCTR_0_EN       = 0x00650 >> 2,
		CORE_PCTR_0_CLR      = 0x00654 >> 2,
		CORE_PCTR_0_OVERFLOW = 0x00658 >> 2,
	
		CORE_PCTR_0_SRC_0    = 0x00660 >> 2,
		CORE_PCTR_0_SRC_1,
		CORE_PCTR_0_SRC_2,
		CORE_PCTR_0_SRC_3,
		CORE_PCTR_0_SRC_4,
		CORE_PCTR_0_SRC_5,
		CORE_PCTR_0_SRC_6,
		CORE_PCTR_0_SRC_7,

		CORE_PCTR_0_PCTR0,
		CORE_PCTR_0_PCTR1,
		CORE_PCTR_0_PCTR2,
		CORE_PCTR_0_PCTR3,
		CORE_PCTR_0_PCTR4,
		CORE_PCTR_0_PCTR5,
		CORE_PCTR_0_PCTR6,
		CORE_PCTR_0_PCTR7,
		CORE_PCTR_0_PCTR8,
		CORE_PCTR_0_PCTR9,
		CORE_PCTR_0_PCTR10,
		CORE_PCTR_0_PCTR11,
		CORE_PCTR_0_PCTR12,
		CORE_PCTR_0_PCTR13,
		CORE_PCTR_0_PCTR14,
		CORE_PCTR_0_PCTR15,
		CORE_PCTR_0_PCTR16,
		CORE_PCTR_0_PCTR17,
		CORE_PCTR_0_PCTR18,
		CORE_PCTR_0_PCTR19,
		CORE_PCTR_0_PCTR20,
		CORE_PCTR_0_PCTR21,
		CORE_PCTR_0_PCTR22,
		CORE_PCTR_0_PCTR23,
		CORE_PCTR_0_PCTR24,
		CORE_PCTR_0_PCTR25,
		CORE_PCTR_0_PCTR26,
		CORE_PCTR_0_PCTR27,
		CORE_PCTR_0_PCTR28,
		CORE_PCTR_0_PCTR29,
		CORE_PCTR_0_PCTR30,
		CORE_PCTR_0_PCTR31,

		CORE_PCTR_COUNT = 32,  // number of PCTR registers 
	};


	~RegisterMapping();

	void init();

	unsigned num_cores(); 

	static RegisterMapping &instance();

	struct Info {
		unsigned num_cores = 0; 

		// version numbers, in given order
		unsigned tver          = 0;
		unsigned rev           = 0;
		unsigned iprev         = 0;
		unsigned ipidx         = 0;

		bool     mmu           = false;
		bool     tfu           = false;
		bool     tsy           = false;
		bool     mso           = false;
		bool     l3c           = false;
		unsigned l3c_nkb       = 0;     // Size of L3C in KB
	};

	struct CoreInfo {
		unsigned ver           = 0;
		unsigned rev           = 0;
		unsigned index         = 0;
		unsigned vpm_size      = 0;
		unsigned num_slice     = 0;
		unsigned num_semaphore = 0;
		unsigned num_tmu       = 0;  // all slices
		unsigned num_qpu       = 0;  // all slices
		bool     bcg_int       = false;
		bool     override_tmu  = false;
	};

	struct Stats {
		static int const NUM_COUNTERS = 32;

		uint32_t counters[NUM_COUNTERS];

		uint32_t gmp_status   = 10;  // init intentionally to non-zero
		uint32_t csd_status   = 10;
		uint32_t fdbg0        = 10;
		uint32_t fdbgb        = 10;
		uint32_t fdbgr        = 10;
		uint32_t fdbgs        = 10;
		uint32_t stat         = 10;
		uint32_t mmuc_control = 10;
		uint32_t mmu_ctl      = 10;

		struct Mmu_Ctl {
			bool cap_exceeded              = false;
			bool cap_exceeded_abort        = false;
			bool cap_exceeded_int          = false;
			bool cap_exceeded_exception    = false;
			bool pt_invalid                = false;
			bool pt_invalid_abort          = false;
			bool pt_invalid_int            = false;
			bool pt_invalid_exception      = false;
			bool pt_invalid_enable         = false;
			bool write_violation           = false;
			bool write_violation_abort     = false;
			bool write_violation_int       = false;
			bool write_violation_exception = false;
			bool tlb_clearing              = false;
			bool tlb_stats_clear           = false;
			bool tlb_clear                 = false;
			bool tlb_stats_enable          = false;
			bool enable                    = false;
		} mmu_ctl_fields;
	};

	Info info();
	CoreInfo info_per_core(unsigned core_index);

	Stats stats();
	void reset_v3d();
	void write(uint32_t offset, uint32_t val) { v3d_write(offset, val); }
	uint32_t core_read(int core_index, uint32_t offset);
	void core_write(int core_index, uint32_t offset, uint32_t val) { v3d_core_write(core_index, offset, val); }

private:
  uint32_t *m_addr = nullptr;
	unsigned  m_size = 0x4000;

	unsigned  m_ncores     = 1;
  uint32_t *map_cores[1] = { nullptr };

	uint32_t v3d_bridge_read(uint32_t offset);
	void     v3d_bridge_write(uint32_t offset, uint32_t val);
	void     v3d_core_write(int core_index, uint32_t offset, uint32_t val);
	uint32_t v3d_read(uint32_t offset);
	void     v3d_write(uint32_t offset, uint32_t val);
	void     v3d_reset_v3d();
	void     v3d_irq_enable();
};


}  // v3d
}  // V3DLib

#endif  // _VC6_REGISTERMAPPING_H
