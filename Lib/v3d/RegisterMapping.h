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

	Info info();
	CoreInfo info_per_core(unsigned core_index);

private:
  uint32_t *m_addr = nullptr;
	unsigned  m_size = 0x4000;

	unsigned  m_ncores     = 1;
  uint32_t *map_cores[1] = { nullptr }; 
};


}  // v3d
}  // QPULib

#endif  // _VC6_REGISTERMAPPING_H
