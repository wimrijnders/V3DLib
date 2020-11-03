#ifndef _VC6_REGISTERMAPPING_H
#define _VC6_REGISTERMAPPING_H
#include <stdint.h>


namespace QPULib {
namespace v3d {

class RegisterMapping {
public:
	~RegisterMapping();

	void init();

	unsigned num_cores(); 

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

private:
  uint32_t *m_addr = nullptr;
	unsigned  m_size = 0x4000;

	unsigned  m_ncores     = 1;
  uint32_t *map_cores[1] = { nullptr };

	uint32_t v3d_bridge_read(uint32_t offset);
	void     v3d_bridge_write(uint32_t offset, uint32_t val);
	void     v3d_core_write(int core, uint32_t offset, uint32_t val);
	void     v3d_write(uint32_t offset, uint32_t val);
};


}  // v3d
}  // QPULib

#endif  // _VC6_REGISTERMAPPING_H
